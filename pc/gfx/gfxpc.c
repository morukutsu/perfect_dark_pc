#include "gfxpc.h"

#include "lib/mtx.h"
#include "print.h"
#include "hashmap.h"
#include "byteswap.h"
#include "native_functions.h"

#include "va_list.h"
#include <stdio.h>
#include <assert.h>

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#include "linmath.h"

#include <signal.h>

// GFXPC Renderer Debug Options
#define GFXPC_DBG_WIREFRAME 0         // Display all primitives as wireframe
#define GFXPC_DBG_COMMANDLIST 0       // Logs all command lists (creates a lot of logs)
#define GFXPC_DBG_EXTRADATA_MAP 0     // Displays the content of the GDL extradata each frame
#define GFXPC_DBG_DISABLE_TEXTURES 0  // Disables textures
#define GFXPC_DBG_TEXTURES_COLOR 0    // Replaces each unique texture by a color

#define GFXPC_DBG_MATRICES 0          // Debug output matrices after a matrix state change
#define GFXPC_DEBUG_CC 0              // Display color combiner settings

// TODO: add a debug option to disable the texture cache

char* vertex_shader;
char* fragment_shader;
static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;

typedef enum t_attrib_id
{
    attrib_position,
    attrib_color,
    attrib_uv
} t_attrib_id;

/* N64 stuff */
// SCALE_M_N: upscale/downscale M-bit integer to N-bit
#define SCALE_5_8(VAL_) (((VAL_) * 0xFF) / 0x1F)
#define SCALE_8_5(VAL_) ((((VAL_) + 4) * 0x1F) / 0xFF)
#define SCALE_4_8(VAL_) ((VAL_) * 0x11)
#define SCALE_8_4(VAL_) ((VAL_) / 0x11)
#define SCALE_3_8(VAL_) ((VAL_) * 0x24)
#define SCALE_8_3(VAL_) ((VAL_) / 0x24)

struct RGBA {
    uint8_t r, g, b, a;
};

struct Combiner {
    uint32_t a, b, c, d;
};

#define VTX_ELEM_COUNT 10
struct Vertex {
    union {
        struct {
            float r, g, b, a, x, y, z, w, u, v;
        };
        float f[VTX_ELEM_COUNT];
    };
};

struct RDPTile {
    uint8_t fmt;
    uint8_t siz;
    uint8_t cms, cmt;
    uint16_t uls, ult, lrs, lrt; // U10.2
    uint32_t line_size_bytes;
    uint32_t tmem_bytes;
};

struct RDPTextureDRAM {
    uint8_t *addr;
    uint8_t siz;
    uint8_t tile_number;
};

struct RDPTextureLoadInfo {
    uint8_t *addr;
    uint32_t size_bytes;
};

struct RDP {
    const uint8_t *palette;
    struct RGBA prim_color, fill_color;
    struct Combiner comb1Color, comb1Alpha, comb2Color, comb2Alpha;
    uint32_t other_mode_l, other_mode_h;

    struct RDPTile tiles[8];
    struct RDPTextureDRAM texture_to_load;
    struct RDPTextureLoadInfo texture_load_info[8];
};

struct RSPMtx {
    float v[4][4];
};

#define MODELVIEW_STACK_SIZE 10

struct RSP {
    void* segments[16];
    struct Vertex vertices[16];
    struct RSPMtx projection_mtx;
    struct RSPMtx modelview_mtx[MODELVIEW_STACK_SIZE];
    struct RSPMtx mp;
    uint32_t modelviewStackSize;
    uint32_t colors[256];
    uint32_t geometry_mode;
    //float current_lookat_coeffs[2][3]; // lookat_x, lookat_y
    uint32_t perspnorm;

    // Lighting
    Light_t lookat_x, lookat_y;
    bool lookatEnable;

    // Textures
    uint32_t tex_s, tex_t, tex_level, tex_tile, tex_on;
};

struct RDP rdp;
struct RSP rsp;

#define C0(pos, width) ((gdl->words.w0 >> (pos)) & ((1U << width) - 1))
#define C1(pos, width) ((gdl->words.w1 >> (pos)) & ((1U << width) - 1))

// Create a hashmap to map a display list slot to additional storage
struct hashmap *gfxpcExtradataMap = NULL;

// TextureCache
struct TextureCache {
    struct hashmap* textures;
};

struct Texture {
    // Hashed fields
    void* addr;
    uint32_t fmt, siz;

    // Non hashed fields
    uint32_t width, height;
    GLuint texture;
};

struct TextureCache textureCache;

uint64_t texture_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct Texture *data = item;
    return hashmap_sip(item, sizeof(void*) + sizeof(uint32_t) * 2, seed0, seed1);
}

int texture_compare(const void *a, const void *b, void *udata) {
    const struct Texture *ua = a;
    const struct Texture *ub = b;

    if (ua->addr == ub->addr && ua->fmt == ub->fmt && ua->siz == ub->siz) {
        return 0;
    }

    return 1;
}

uint64_t extradata_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct cmd_extradata *data = item;
    return hashmap_sip(&data->cmdPtr, 8, seed0, seed1);
}

int extradata_compare(const void *a, const void *b, void *udata) {
    const struct cmd_extradata *ua = a;
    const struct cmd_extradata *ub = b;
    return (uintptr_t)ua->cmdPtr - (uintptr_t)ub->cmdPtr;
}

void gfx_extradata_clear()
{
    hashmap_clear(gfxpcExtradataMap, false);
}

GLuint vs, fs, program;
GLuint vao, vbo;

GLuint rectVao, rectVbo;

GLuint tex, dummyTexture;

int frames = 0;

int gfx_create_shader(GLuint glShaderType, const char* shaderSrc, GLuint* shaderOut)
{
    GLuint shader = glCreateShader(glShaderType);

    GLint shaderSrcLength = (GLint)strlen(shaderSrc);

    glShaderSource(shader, 1, (const GLchar **)&shaderSrc, &shaderSrcLength);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        debugPrint(PC_DBG_FLAG_GFX, "[SHADER] compilation failed!\n");

        GLint logSize = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

        const GLint ERR_LOG_MAXSIZE = 2048;
        char errLogBuffer[ERR_LOG_MAXSIZE];
        GLsizei errLogSize;
        glGetShaderInfoLog(shader, ERR_LOG_MAXSIZE, &errLogSize, errLogBuffer);

        debugPrint(PC_DBG_FLAG_GFX, "- %s\n", errLogBuffer);

        return 1;
    }

    *shaderOut = shader;

    return 0;
}

char* gfx_pc_read_text_file(const char* path)
{
    FILE* f = fopen(path, "r");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        unsigned long len = (unsigned long)ftell(f);
        fseek(f, 0, SEEK_SET);

        char* out = nativeMalloc(len + 1);
        fread(out, len, 1, f);
        out[len - 1] = 0;

        fclose(f);
        return out;
    }

    return NULL;
}

void gfx_create_dummy_texture()
{
    uint32_t pixels[8 * 8];
    for (int i = 0; i < 8 * 8; i++) pixels[i] = 0xFFFFFFFF;

    glGenTextures(1, &dummyTexture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dummyTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void gfx_pc_init()
{
    gfxpcExtradataMap = hashmap_new(sizeof(struct cmd_extradata), 0, 0, 0, extradata_hash, extradata_compare, NULL, NULL);
    textureCache.textures = hashmap_new(sizeof(struct Texture), 0, 0, 0, texture_hash, texture_compare, NULL, NULL);

    // Load shaders from file
    vertex_shader = gfx_pc_read_text_file("shaders/vert.glsl");
    fragment_shader = gfx_pc_read_text_file("shaders/frag.glsl");

    // OGL
    debugPrint(PC_DBG_FLAG_GFX, "Compile vertex shader\n");
    if (gfx_create_shader(GL_VERTEX_SHADER, (const char*)vertex_shader, &vs))
        fatalExit();

    debugPrint(PC_DBG_FLAG_GFX, "Compile fragment shader\n");
    if (gfx_create_shader(GL_FRAGMENT_SHADER, (const char*)fragment_shader, &fs))
        fatalExit();

    nativeFree(vertex_shader);
    nativeFree(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vs );
    glAttachShader(program, fs );

    glBindAttribLocation(program, attrib_position, "i_position");
    glBindAttribLocation(program, attrib_color, "i_color");
    glBindAttribLocation(program, attrib_uv, "i_uv");

    glLinkProgram(program);

    glUseProgram(program);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    mat4x4 projection_matrix;
    mat4x4_ortho(projection_matrix, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix" ), 1, GL_FALSE, (const GLfloat*)projection_matrix);

    // Create simple shader for drawing rectangles?
    glGenVertexArrays(1, &rectVao);
    glGenBuffers(1, &rectVbo);
    glBindVertexArray(rectVao);
    glBindBuffer(GL_ARRAY_BUFFER, rectVbo);

    glEnableVertexAttribArray(attrib_position);
    glEnableVertexAttribArray(attrib_color);
    glEnableVertexAttribArray(attrib_uv);

    glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(float) * VTX_ELEM_COUNT, 0 );
    glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(float) * VTX_ELEM_COUNT, (void*)(4 * sizeof(float)) );
    glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(float) * VTX_ELEM_COUNT, (void*)(8 * sizeof(float)) );

    // temp
    glGenTextures(1, &tex);
    gfx_create_dummy_texture();
}

void upload_texture(const uint8_t *rgba32_buf, int width, int height, struct Texture* texture) {
    glGenTextures(1, &texture->texture);
    texture->width = width;
    texture->height = height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba32_buf);
}

void import_texture_ci4(int tile, struct Texture* texture) {
    uint8_t rgba32_buf[32768];
    
    //print("- import_texture_ci4 addr: %016llx\n", rdp.texture_load_info[tile].addr);
    u32 tlut_ia16 = (rdp.other_mode_h & (3U << G_MDSFT_TEXTLUT)) == G_TT_IA16;
    u32 tlut_rgba = (rdp.other_mode_h & (3U << G_MDSFT_TEXTLUT)) == G_TT_RGBA16;

    if (tlut_ia16) 
    {
        // FMT: iiiiiii aaaaaaaa
        for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes * 2; i++) {
            uint8_t byte = rdp.texture_load_info[tile].addr[i / 2];
            uint8_t idx = (byte >> (4 - (i % 2) * 4)) & 0xf;

            uint16_t col16 = (rdp.palette[idx * 2] << 8) | rdp.palette[idx * 2 + 1]; // Big endian load

            uint8_t a = (col16 >> 8) & 0xFF;
            uint8_t r = (col16) & 0xFF; 
            uint8_t g = (col16) & 0xFF;
            uint8_t b = (col16) & 0xFF;

            rgba32_buf[4*i + 0] = r;
            rgba32_buf[4*i + 1] = g;
            rgba32_buf[4*i + 2] = b;
            rgba32_buf[4*i + 3] = a;
        }
    } 
    else 
    {
        if (tlut_rgba)
        {
            for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes * 2; i++) {
                uint8_t byte = rdp.texture_load_info[tile].addr[i / 2];
                uint8_t idx = (byte >> (4 - (i % 2) * 4)) & 0xf;

                uint16_t col16 = (rdp.palette[idx * 2] << 8) | rdp.palette[idx * 2 + 1]; // Big endian load

                uint8_t a = col16 & 1;
                uint8_t r = col16 >> 11;
                uint8_t g = (col16 >> 6) & 0x1f;
                uint8_t b = (col16 >> 1) & 0x1f;

                rgba32_buf[4*i + 0] = SCALE_5_8(r);
                rgba32_buf[4*i + 1] = SCALE_5_8(g);
                rgba32_buf[4*i + 2] = SCALE_5_8(b);
                rgba32_buf[4*i + 3] = a ? 255 : 0;
            }
        }
        else
        {
            debugPrint(PC_DBG_FLAG_GFX, "TODO unhandled import_texture_ci4 mode\n");
            //exit(1);
        }
    }

    uint32_t width = rdp.tiles[tile].line_size_bytes * 2;
    uint32_t height = rdp.texture_load_info[tile].size_bytes / rdp.tiles[tile].line_size_bytes;
    
    upload_texture(rgba32_buf, width, height, texture);
}

void import_texture_ia4(int tile, struct Texture* texture) {
    uint8_t rgba32_buf[32768];
    
    for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes * 2; i++) {
        uint8_t byte = rdp.texture_load_info[tile].addr[i / 2];
        uint8_t part = (byte >> (4 - (i % 2) * 4)) & 0xf;
        uint8_t intensity = part >> 1;
        uint8_t alpha = part & 1;
        uint8_t r = intensity;
        uint8_t g = intensity;
        uint8_t b = intensity;
        rgba32_buf[4*i + 0] = SCALE_3_8(r);
        rgba32_buf[4*i + 1] = SCALE_3_8(g);
        rgba32_buf[4*i + 2] = SCALE_3_8(b);
        rgba32_buf[4*i + 3] = alpha ? 255 : 0;
    }
    
    uint32_t width = rdp.tiles[tile].line_size_bytes * 2;
    uint32_t height = rdp.texture_load_info[tile].size_bytes / rdp.tiles[tile].line_size_bytes;
    
    upload_texture(rgba32_buf, width, height, texture);
}

void import_texture_i4(int tile, struct Texture* texture) {
    uint8_t rgba32_buf[32768];

    for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes * 2; i++) {
        uint8_t byte = rdp.texture_load_info[tile].addr[i / 2];
        uint8_t part = (byte >> (4 - (i % 2) * 4)) & 0xf;
        uint8_t intensity = part;
        uint8_t r = intensity;
        uint8_t g = intensity;
        uint8_t b = intensity;
        rgba32_buf[4*i + 0] = SCALE_4_8(r);
        rgba32_buf[4*i + 1] = SCALE_4_8(g);
        rgba32_buf[4*i + 2] = SCALE_4_8(b);
        rgba32_buf[4*i + 3] = 255;
    }

    uint32_t width = rdp.tiles[tile].line_size_bytes * 2;
    uint32_t height = rdp.texture_load_info[tile].size_bytes / rdp.tiles[tile].line_size_bytes;
    
    upload_texture(rgba32_buf, width, height, texture);
}

static void import_texture_ia8(int tile, struct Texture* texture) {
    uint8_t rgba32_buf[16384];
    
    for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes; i++) {
        uint8_t intensity = rdp.texture_load_info[tile].addr[i] >> 4;
        uint8_t alpha = rdp.texture_load_info[tile].addr[i] & 0xf;
        uint8_t r = intensity;
        uint8_t g = intensity;
        uint8_t b = intensity;
        rgba32_buf[4*i + 0] = SCALE_4_8(r);
        rgba32_buf[4*i + 1] = SCALE_4_8(g);
        rgba32_buf[4*i + 2] = SCALE_4_8(b);
        rgba32_buf[4*i + 3] = SCALE_4_8(alpha);
    }
    
    uint32_t width = rdp.tiles[tile].line_size_bytes;
    uint32_t height = rdp.texture_load_info[tile].size_bytes / rdp.tiles[tile].line_size_bytes;

    upload_texture(rgba32_buf, width, height, texture);
}

void import_texture_ci8(int tile, struct Texture* texture) 
{
    uint8_t rgba32_buf[16384];
    
    for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes; i++) {
        uint8_t idx = rdp.texture_load_info[tile].addr[i];
        uint16_t col16 = (rdp.palette[idx * 2] << 8) | rdp.palette[idx * 2 + 1]; // Big endian load
        uint8_t a = col16 & 1;
        uint8_t r = col16 >> 11;
        uint8_t g = (col16 >> 6) & 0x1f;
        uint8_t b = (col16 >> 1) & 0x1f;
        rgba32_buf[4*i + 0] = SCALE_5_8(r);
        rgba32_buf[4*i + 1] = SCALE_5_8(g);
        rgba32_buf[4*i + 2] = SCALE_5_8(b);
        rgba32_buf[4*i + 3] = a ? 255 : 0;
    }
    
    uint32_t width = rdp.tiles[tile].line_size_bytes;
    uint32_t height = rdp.texture_load_info[tile].size_bytes / rdp.tiles[tile].line_size_bytes;

    upload_texture(rgba32_buf, width, height, texture);
}

void import_texture_i8(int tile, struct Texture* texture) 
{
    uint8_t rgba32_buf[16384];

    for (uint32_t i = 0; i < rdp.texture_load_info[tile].size_bytes; i++) {
        uint8_t intensity = rdp.texture_load_info[tile].addr[i];
        uint8_t r = intensity;
        uint8_t g = intensity;
        uint8_t b = intensity;

        rgba32_buf[4*i + 0] = r;
        rgba32_buf[4*i + 1] = g;
        rgba32_buf[4*i + 2] = b;
        rgba32_buf[4*i + 3] = 255;
    }

    uint32_t width = rdp.tiles[tile].line_size_bytes;
    uint32_t height = rdp.texture_load_info[tile].size_bytes / rdp.tiles[tile].line_size_bytes;

    upload_texture(rgba32_buf, width, height, texture);
}

void gfx_get_texture_size(uint32_t tile, uint32_t* w, uint32_t* h)
{
    uint32_t ww = 0, hh = 0;

    uint8_t fmt = rdp.tiles[tile].fmt;
    uint8_t siz = rdp.tiles[tile].siz;
    uint8_t rshift = 0, lshift = 0;

    if (siz == G_IM_SIZ_8b) {
    } else if (siz == G_IM_SIZ_4b) {
        lshift = 1;
    } else if (siz == G_IM_SIZ_16b) {
        rshift = 1;
    } else {
        debugPrint(PC_DBG_FLAG_GFX, "ERROR gfx_get_texture_size unsupported fmt\n");
        fatalExit();
    }

    ww = rdp.tiles[tile].line_size_bytes;

    if (rshift)
        ww = ww >> rshift;
    else if (lshift)
        ww = ww << lshift;

    uint32_t size_bytes = 0;
    if (rsp.tex_level == 0) {
        size_bytes = rdp.texture_load_info[tile].size_bytes;
    } else {
        uint32_t tile_ww = 0, tile_hh = 0;
        tile_ww = (rdp.tiles[tile].lrs - rdp.tiles[tile].uls + 4) / 4;
        tile_hh = (rdp.tiles[tile].lrt - rdp.tiles[tile].ult + 4) / 4;

        size_bytes = tile_ww * tile_hh;

        // Swapping lshift and rshift because here we convert from texels to bytes
        if (rshift)
            size_bytes = size_bytes << rshift;
        else if (lshift)
            size_bytes = size_bytes >> lshift;
    }

    hh = size_bytes / rdp.tiles[tile].line_size_bytes;

    *w = ww;
    *h = hh;
}

void import_texture_rgba16(int tile, struct Texture* texture)
{
    uint8_t rgba32_buf[8192];
    uint32_t ww, hh;
    
    gfx_get_texture_size(tile, &ww, &hh);
    uint32_t size_bytes = ww * hh * 2;

    for (uint32_t i = 0; i < size_bytes / 2; i++) {
        uint16_t col16 = (rdp.texture_load_info[tile].addr[2 * i] << 8) | rdp.texture_load_info[tile].addr[2 * i + 1];
        uint8_t a = col16 & 1;
        uint8_t r = col16 >> 11;
        uint8_t g = (col16 >> 6) & 0x1f;
        uint8_t b = (col16 >> 1) & 0x1f;
        rgba32_buf[4*i + 0] = SCALE_5_8(r);
        rgba32_buf[4*i + 1] = SCALE_5_8(g);
        rgba32_buf[4*i + 2] = SCALE_5_8(b);
        rgba32_buf[4*i + 3] = a ? 255 : 0;
    }
    
    upload_texture(rgba32_buf, ww, hh, texture);
}

void import_texture_dbg_color(int tile, struct Texture* texture)
{
    uint8_t rgba32_buf[32768];
    uint32_t ww, hh;
    
    gfx_get_texture_size(tile, &ww, &hh);
    uint32_t size_bytes = ww * hh * 2;

    //uint64_t seed = rdp.texture_load_info[tile].addr;
    uint64_t seed1 = rand();
    uint64_t seed2 = rand();
    uint64_t seed3 = rand();

    for (uint32_t i = 0; i < size_bytes / 2; i++) {
        rgba32_buf[4*i + 0] = seed1 % 256;
        rgba32_buf[4*i + 1] = seed2 % 256;
        rgba32_buf[4*i + 2] = seed3 % 256;
        rgba32_buf[4*i + 3] = 255;
    }
    
    upload_texture(rgba32_buf, ww, hh, texture);
}

struct Texture* import_texture(int tile) {
    uint8_t fmt = rdp.tiles[tile].fmt;
    uint8_t siz = rdp.tiles[tile].siz;
    
    // Check if the texture is in the cache
    // For now we use addr and tile parameters. Could be safer to hash the pixels
    struct Texture tex = {rdp.texture_load_info[tile].addr, fmt, siz };
    void* item = hashmap_get(textureCache.textures, &tex);
    if (item) {
        //print("Found texture %llx %d %d\n", tex.addr, tex.fmt, tex.siz);
        return (struct Texture*)item;
    } else {
        //print("New texture: %llx %d %d\n", tex.addr, tex.fmt, tex.siz);
    }

#if GFXPC_DBG_TEXTURES_COLOR
    import_texture_dbg_color(tile, &tex);
    hashmap_set(textureCache.textures, &tex);
    return hashmap_get(textureCache.textures, &tex);
#endif

    if (fmt == G_IM_FMT_RGBA) {
        if (siz == G_IM_SIZ_16b) {
            import_texture_rgba16(tile, &tex);
        } else if (siz == G_IM_SIZ_32b) {
            print("unhandled import_texture fmt: %d siz: %d\n", fmt, siz);
            //import_texture_rgba32(tile);
        } else {
            //abort();
        }
    } else if (fmt == G_IM_FMT_IA) {
        if (siz == G_IM_SIZ_4b) {
            import_texture_ia4(tile, &tex);
        } else if (siz == G_IM_SIZ_8b) {
            import_texture_ia8(tile, &tex);
        } else if (siz == G_IM_SIZ_16b) {
            print("unhandled import_texture fmt: %d siz: %d\n", fmt, siz);
            //import_texture_ia16(tile);
        } else {
        }
    } else if (fmt == G_IM_FMT_CI) {
        if (siz == G_IM_SIZ_4b) {
            import_texture_ci4(tile, &tex);
        } else if (siz == G_IM_SIZ_8b) {
            import_texture_ci8(tile, &tex);
        } else {
            //abort();
        }
    } else if (fmt == G_IM_FMT_I) {
        if (siz == G_IM_SIZ_4b) {
            import_texture_i4(tile, &tex);
        } else if (siz == G_IM_SIZ_8b) {
            import_texture_i8(tile, &tex);
        } else {
            //abort();
        }
    } else {
        //abort();
    }

    hashmap_set(textureCache.textures, &tex);
    return hashmap_get(textureCache.textures, &tex);
}

void vtx_copy(struct Vertex* src, float* dst)
{
    for (int i = 0; i < VTX_ELEM_COUNT; i++) 
        dst[i] = src->f[i];
}

#define FX_10_2_TO_F(v) ((float)(v) / 4.0f)
#define FX_10_5_TO_F(v) ((float)(v) / 32.0f)
#define FX_5_10_TO_F(v) ((float)(v) / 1024.0f)

void gfx_dp_texture_rectangle(int32_t ulx, int32_t uly, int32_t lrx, int32_t lry, uint8_t tile, int16_t uls, int16_t ult, int16_t dsdx, int16_t dtdy, bool flip) {
    struct Texture* texture = import_texture(tile);

    // Convert coordinates from 10.2 to float (divide by 4)
    // Note: handle offsets for copy mode
    float ulxf = FX_10_2_TO_F(ulx);
    float ulyf = FX_10_2_TO_F(uly);
    float lrxf = FX_10_2_TO_F(lrx);
    float lryf = FX_10_2_TO_F(lry);

    float r, g, b, a;
    r = g = b = 1.0f;

    // uls and ult are S10.5
    // dsdx and dtdy are S5.10
    // lrx, lry, ulx, uly are U10.2
    // lrs, lrt are S10.5
    if (flip) {
        dsdx = -dsdx;
        dtdy = -dtdy;
    }

    // To calculate the UVs of the bottom right corner, we need to increase the top left
    // UVs using dsdx and dtdy (1 pixel moves [U,V] by [dsdx,dsdy])
    float rectWidth = FX_10_2_TO_F(!flip ? lrx - ulx : lry - uly);
    float rectHeight = FX_10_2_TO_F(!flip ? lry - uly : lrx - ulx);
    float ulsf = FX_10_5_TO_F(uls);
    float ultf = FX_10_5_TO_F(ult);

    float lrs = ulsf + FX_5_10_TO_F(dsdx) * rectWidth;
    float lrt = ultf + FX_5_10_TO_F(dtdy) * rectHeight;

    struct Vertex vtx[] = {
        { r, g, b, 1, ulxf, ulyf, 0, 1.0, ulsf, ultf },
        { r, g, b, 1, lrxf, ulyf, 0, 1.0, lrs, ultf },
        { r, g, b, 1, lrxf, lryf, 0, 1.0, lrs, lrt },
        { r, g, b, 1, ulxf, lryf, 0, 1.0, ulsf, lrt }
    };

    uint32_t ww = 0, hh = 0;
    gfx_get_texture_size(tile, &ww, &hh);

    // Scale UVs to 0, 1
    for (int i = 0; i < 4; i++) {
        vtx[i].u = vtx[i].u / ww;
        vtx[i].v = vtx[i].v / hh;
    }

    GLfloat g_vertex_buffer_data[6 * VTX_ELEM_COUNT];

    vtx_copy(&vtx[0], &g_vertex_buffer_data[0 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[1], &g_vertex_buffer_data[1 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[2], &g_vertex_buffer_data[2 * VTX_ELEM_COUNT]);

    vtx_copy(&vtx[0], &g_vertex_buffer_data[3 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[2], &g_vertex_buffer_data[4 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[3], &g_vertex_buffer_data[5 * VTX_ELEM_COUNT]);

    // Display
    glBindBuffer(GL_ARRAY_BUFFER, rectVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->texture);

    mat4x4 projection_matrix;
    mat4x4_ortho(projection_matrix, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 100.0f);
    
    mat4x4 identity_matrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix" ), 1, GL_FALSE, (const GLfloat *)projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_modelview_matrix" ), 1, GL_FALSE, (const GLfloat *)identity_matrix);

    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    {
        GLfloat c[4] = { 
            rdp.prim_color.r / 255.0f,
            rdp.prim_color.g / 255.0f,
            rdp.prim_color.b / 255.0f,
            rdp.prim_color.a / 255.0f,
        };

        glUniform4f(glGetUniformLocation(program, "rdpPrimColor"), c[0], c[1], c[2], c[3]);

    }
    
    // Configure the Color Combiner
    {
        GLfloat cc_1[8] = { 
            rdp.comb1Color.a, rdp.comb1Color.b, rdp.comb1Color.c, rdp.comb1Color.d, 
            rdp.comb1Alpha.a, rdp.comb1Alpha.b, rdp.comb1Alpha.c, rdp.comb1Alpha.d 
        };

        glUniform1fv(glGetUniformLocation(program, "cc_1"), 8, cc_1);
    }

    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(rectVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void gfx_dp_fill_rectangle(int32_t ulx, int32_t uly, int32_t lrx, int32_t lry) 
{
    float r, g, b, a;

    uint32_t mode = (rdp.other_mode_h & (3U << G_MDSFT_CYCLETYPE));
    if (mode == G_CYC_FILL) {
        // Per documentation one extra pixel is added in this modes to each edge
        lrx += 1 << 2;
        lry += 1 << 2;

        r = rdp.fill_color.r / 255.0;
        g = rdp.fill_color.g / 255.0;
        b = rdp.fill_color.b / 255.0;
        a = rdp.fill_color.a / 255.0;
    } else {
        r = rdp.prim_color.r / 255.0;
        g = rdp.prim_color.g / 255.0;
        b = rdp.prim_color.b / 255.0;
        a = rdp.prim_color.a / 255.0;
    }

    float ulxf = (ulx);
    float ulyf = (uly);
    float lrxf = (lrx);
    float lryf = (lry);

    struct Vertex vtx[] = {
        { r, g, b, a, ulxf, ulyf, 0, 1.0, 0, 0 },
        { r, g, b, a, lrxf, ulyf, 0, 1.0, 0, 0 },
        { r, g, b, a, lrxf, lryf, 0, 1.0, 0, 0 },
        { r, g, b, a, ulxf, lryf, 0, 1.0, 0, 0 }
    };

    GLfloat g_vertex_buffer_data[6 * VTX_ELEM_COUNT];

    vtx_copy(&vtx[0], &g_vertex_buffer_data[0 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[1], &g_vertex_buffer_data[1 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[2], &g_vertex_buffer_data[2 * VTX_ELEM_COUNT]);

    vtx_copy(&vtx[0], &g_vertex_buffer_data[3 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[2], &g_vertex_buffer_data[4 * VTX_ELEM_COUNT]);
    vtx_copy(&vtx[3], &g_vertex_buffer_data[5 * VTX_ELEM_COUNT]);

    // Display
    glBindBuffer(GL_ARRAY_BUFFER, rectVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dummyTexture);

    mat4x4 projection_matrix;
    mat4x4_ortho(projection_matrix, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 100.0f);
    
    mat4x4 identity_matrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix" ), 1, GL_FALSE, (const GLfloat *)projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_modelview_matrix" ), 1, GL_FALSE, (const GLfloat *)identity_matrix);

    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    {
        GLfloat c[4] = { 
            rdp.prim_color.r / 255.0f,
            rdp.prim_color.g / 255.0f,
            rdp.prim_color.b / 255.0f,
            rdp.prim_color.a / 255.0f,
        };

        glUniform4f(glGetUniformLocation(program, "rdpPrimColor"), c[0], c[1], c[2], c[3]);

    }
    
    // Configure the Color Combiner
    {
        GLfloat cc_1[8] = { 
            rdp.comb1Color.a, rdp.comb1Color.b, rdp.comb1Color.c, rdp.comb1Color.d, 
            rdp.comb1Alpha.a, rdp.comb1Alpha.b, rdp.comb1Alpha.c, rdp.comb1Alpha.d 
        };

        glUniform1fv(glGetUniformLocation(program, "cc_1"), 8, cc_1);
    }

    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(rectVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void gfx_sp_tri4(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2, uint8_t x3, uint8_t y3, uint8_t z3, uint8_t x4, uint8_t y4, uint8_t z4)
{
    //print("tri %x %x %x - %x %x %x - %x %x %x - %x %x %x\n", x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    
    uint32_t mode = (rdp.other_mode_h & (3U << G_MDSFT_CYCLETYPE));
    uint32_t lod = (rdp.other_mode_h & (2U << G_MDSFT_TEXTLOD));

    // TODO: if there's a mipmap the texture should be imported differently (use the tile )
    if (rsp.tex_level > 0) { } else { }

    // Check if texturing is enabled before importing textures
    struct Texture* texture = NULL;
    if (rsp.tex_on) {
        texture = import_texture(rsp.tex_tile);
    }

    // Get the texture size (loaded in opengl)
    uint32_t ww = 0, hh = 0;
    gfx_get_texture_size(rsp.tex_tile, &ww, &hh);

    // Get the texture size (from the tile descriptor)
    uint32_t tile_ww = 0, tile_hh = 0;
    tile_ww = (rdp.tiles[rsp.tex_tile].lrs - rdp.tiles[rsp.tex_tile].uls + 4) / 4;
    tile_hh = (rdp.tiles[rsp.tex_tile].lrt - rdp.tiles[rsp.tex_tile].ult + 4) / 4;

    uint8_t tris[] = {
        x1, y1, z1,
        x2, y2, z2,
        x3, y3, z3,
        x4, y4, z4
    };

    // Build vertex buffer: 4 triangles of 3 vertices
    GLfloat g_vertex_buffer_data[4 * 3 * VTX_ELEM_COUNT];
    uint8_t trianglesCount = 0;
    uint8_t verticesCount = 0;

    for (int k = 0; k < 4; k++) {
        uint8_t v1 = tris[k * 3];
        uint8_t v2 = tris[k * 3 + 1];
        uint8_t v3 = tris[k * 3 + 2];

        if (v1 != 0 || v2 != 0 || v3 != 0) {
            struct Vertex* rspVtx[3] = { &rsp.vertices[v1], &rsp.vertices[v2], &rsp.vertices[v3] };
            
            //print("1 x %f y %f z %f w %f\n", rspVtx[0]->x, rspVtx[0]->y, rspVtx[0]->z, rspVtx[0]->w);
            //print("2 x %f y %f z %f w %f\n", rspVtx[1]->x, rspVtx[1]->y, rspVtx[1]->z, rspVtx[1]->w);
            //print("3 x %f y %f z %f w %f\n\n", rspVtx[2]->x, rspVtx[2]->y, rspVtx[2]->z, rspVtx[2]->w);

            struct Vertex vtx[] = { 
                { rspVtx[0]->r, rspVtx[0]->g, rspVtx[0]->b, rspVtx[0]->a, rspVtx[0]->x, rspVtx[0]->y, rspVtx[0]->z, rspVtx[0]->w, rspVtx[0]->u, rspVtx[0]->v },
                { rspVtx[1]->r, rspVtx[1]->g, rspVtx[1]->b, rspVtx[1]->a, rspVtx[1]->x, rspVtx[1]->y, rspVtx[1]->z, rspVtx[1]->w, rspVtx[1]->u, rspVtx[1]->v }, 
                { rspVtx[2]->r, rspVtx[2]->g, rspVtx[2]->b, rspVtx[2]->a, rspVtx[2]->x, rspVtx[2]->y, rspVtx[2]->z, rspVtx[2]->w, rspVtx[2]->u, rspVtx[2]->v },
            };

            if (texture) {
                for (int i = 0; i < 3; i++) {
                    // Offset UVs using tile size descriptor data
                    // uls/ult are 10.2, uv is float
                    vtx[i].u = vtx[i].u - FX_10_2_TO_F(rdp.tiles[rsp.tex_tile].uls);
                    vtx[i].v = vtx[i].v - FX_10_2_TO_F(rdp.tiles[rsp.tex_tile].uls);

                    if ((rdp.other_mode_h & (3U << G_MDSFT_TEXTFILT)) != G_TF_POINT) {
                        vtx[i].u += 0.5f;
                        vtx[i].v += 0.5f;
                    }

                    // Scale UVs to 0, 1 (in opengl texture space)
                    vtx[i].u = vtx[i].u / ww;
                    vtx[i].v = vtx[i].v / hh;
                }
            }

            for (int i = 0; i < 3; i++)
            {
                vtx_copy(&vtx[i], &g_vertex_buffer_data[verticesCount * VTX_ELEM_COUNT]);
                verticesCount++;
            }

            trianglesCount++;
        } 
    }

    // Display
    glBindBuffer(GL_ARRAY_BUFFER, rectVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

    glActiveTexture(GL_TEXTURE0);

#if GFXPC_DBG_DISABLE_TEXTURES
    glBindTexture(GL_TEXTURE_2D, dummyTexture);
#else
    if (texture) { 
        glBindTexture(GL_TEXTURE_2D, texture->texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, dummyTexture);
    }
#endif

    mat4x4 identity_matrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix" ), 1, GL_FALSE, (const GLfloat *)identity_matrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_modelview_matrix" ), 1, GL_FALSE, (const GLfloat *)identity_matrix);

    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    // Configure the Color Combiner
#if GFXPC_DBG_DISABLE_TEXTURES || GFXPC_DBG_TEXTURES_COLOR
    GLfloat cc_1[8] = { 
        G_CCMUX_1, G_CCMUX_0, G_CCMUX_TEXEL0, G_CCMUX_0, 
        G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE 
    };

    GLfloat cc_2[8] = { 
        G_CCMUX_1, G_CCMUX_0, G_CCMUX_TEXEL0, G_CCMUX_0, 
        G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE 
    };

    rdp.prim_color.r = rdp.prim_color.g = rdp.prim_color.b = rdp.prim_color.a = 255;
#else
    GLfloat cc_1[8] = { 
        rdp.comb1Color.a, rdp.comb1Color.b, rdp.comb1Color.c, rdp.comb1Color.d, 
        rdp.comb1Alpha.a, rdp.comb1Alpha.b, rdp.comb1Alpha.c, rdp.comb1Alpha.d 
    };

    GLfloat cc_2[8] = { 
        rdp.comb2Color.a, rdp.comb2Color.b, rdp.comb2Color.c, rdp.comb2Color.d, 
        rdp.comb2Alpha.a, rdp.comb2Alpha.b, rdp.comb2Alpha.c, rdp.comb2Alpha.d 
    };
#endif

    {
        GLfloat c[4] = { 
            rdp.prim_color.r / 255.0f,
            rdp.prim_color.g / 255.0f,
            rdp.prim_color.b / 255.0f,
            rdp.prim_color.a / 255.0f,
        };

        glUniform4f(glGetUniformLocation(program, "rdpPrimColor"), c[0], c[1], c[2], c[3]);
    }
    
#if GFXPC_DEBUG_CC
    debugPrint(PC_DBG_FLAG_GFX, "cc1 %f %f %f %f - %f %f %f %f\n", cc_1[0], cc_1[1], cc_1[2], cc_1[3], cc_1[4], cc_1[5], cc_1[6], cc_1[7]);
    debugPrint(PC_DBG_FLAG_GFX, "cc2 %f %f %f %f - %f %f %f %f\n", cc_2[0], cc_2[1], cc_2[2], cc_2[3], cc_2[4], cc_2[5], cc_2[6], cc_2[7]);
#endif

    glUniform1fv(glGetUniformLocation(program, "cc_1"), 8, cc_1);
    glUniform1fv(glGetUniformLocation(program, "cc_2"), 8, cc_2);

    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    glDisable(GL_CULL_FACE);
    //glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(rectVao);
    glDrawArrays(GL_TRIANGLES, 0, verticesCount);
}

void gfx_dbg_cube()
{
    // Build vertex buffer: 4 triangles of 3 vertices
    GLfloat g_vertex_buffer_data[12 * 3 * VTX_ELEM_COUNT];
    uint8_t trianglesCount = 0;
    uint8_t verticesCount = 0;

    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    float cw = 5.0f;
    float ch = 5.0f;
    float depth = 5.0f;

    float vertices[] = { 
        -cw, -ch, -depth, // 0
        cw, -ch, -depth, // 1
        cw,  ch, -depth, // 2
        -cw,  ch, -depth, // 3
        -cw, -ch,  depth, // 4
        cw, -ch,  depth, // 5
        cw,  ch,  depth, // 6
        -cw,  ch,  depth // 7
    };  

    short indices[] = { 
        0,2,1,
        0,3,2,
        1,2,6,
        6,5,1,
        4,5,6,
        6,7,4,
        2,3,6,
        6,3,7,
        0,7,3,
        0,4,7,
        0,1,5,
        0,5,4
    }; 
         
    for (int i = 0; i < 12; i++) 
    {
        short ind[3] = { indices[i * 3], indices[i * 3 + 1], indices[i * 3 + 2] };

        struct Vertex vtx[] = { 
            r, g, b, a, vertices[ind[0] * 3 + 0], vertices[ind[0] * 3 + 1], vertices[ind[0] * 3 + 2], 1.0f, 0, 0,
            r, g, b, a, vertices[ind[1] * 3 + 0], vertices[ind[1] * 3 + 1], vertices[ind[1] * 3 + 2], 1.0f, 0, 0, 
            r, g, b, a, vertices[ind[2] * 3 + 0], vertices[ind[2] * 3 + 1], vertices[ind[2] * 3 + 2], 1.0f, 0, 0,
        };

        for (int j = 0; j < 3; j++)
        {
            vtx_copy(&vtx[j], &g_vertex_buffer_data[verticesCount * VTX_ELEM_COUNT]);
            verticesCount++;
        }

        trianglesCount++;
    }

    // Display
    glBindBuffer(GL_ARRAY_BUFFER, rectVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dummyTexture);

    mat4x4 projection_matrix;
    mat4x4_perspective(projection_matrix, 90.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 10000.0f);

    mat4x4 cam_matrix;
    vec3 eye = { 0.0f, 0.0f, 100.0f };
    vec3 center = { 0.0f, 0.0f, 0.0f };
    vec3 up = { 0.0f, 1.0f, 0.0f };

    mat4x4_look_at(cam_matrix, eye, center, up);

    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix" ), 1, GL_FALSE, (const GLfloat *)projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_modelview_matrix" ), 1, GL_FALSE, (const GLfloat *)cam_matrix);

    //glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix" ), 1, GL_FALSE, identity_matrix);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    {
        GLfloat c[4] = { 
            rdp.prim_color.r / 255.0f,
            rdp.prim_color.g / 255.0f,
            rdp.prim_color.b / 255.0f,
            rdp.prim_color.a / 255.0f,
        };

        glUniform4f(glGetUniformLocation(program, "rdpPrimColor"), c[0], c[1], c[2], c[3]);
    }
    
    // Configure the Color Combiner
    {
        GLfloat cc_1[8] = { 
            rdp.comb1Color.a, rdp.comb1Color.b, rdp.comb1Color.c, rdp.comb1Color.d, 
            rdp.comb1Alpha.a, rdp.comb1Alpha.b, rdp.comb1Alpha.c, rdp.comb1Alpha.d 
        };

        glUniform1fv(glGetUniformLocation(program, "cc_1"), 8, cc_1);
    }

    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    //glDisable(GL_CULL_FACE);

    glBindVertexArray(rectVao);
    glDrawArrays(GL_TRIANGLES, 0, verticesCount);
}

static void gfx_dp_set_fill_color(uint32_t packed_color) {
    uint16_t col16 = (uint16_t)packed_color;
    uint32_t r = col16 >> 11;
    uint32_t g = (col16 >> 6) & 0x1f;
    uint32_t b = (col16 >> 1) & 0x1f;
    uint32_t a = col16 & 1;
    rdp.fill_color.r = SCALE_5_8(r);
    rdp.fill_color.g = SCALE_5_8(g);
    rdp.fill_color.b = SCALE_5_8(b);
    rdp.fill_color.a = a * 255;
}

static void gfx_dp_set_prim_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    rdp.prim_color.r = r;
    rdp.prim_color.g = g;
    rdp.prim_color.b = b;
    rdp.prim_color.a = a;
}

static void gfx_dp_set_combine_mode(uint32_t a0, uint32_t b0, uint32_t c0, uint32_t d0,
    uint32_t Aa0, uint32_t Ab0, uint32_t Ac0, uint32_t Ad0,
    uint32_t a1, uint32_t b1, uint32_t c1, uint32_t d1,
    uint32_t Aa1, uint32_t Ab1, uint32_t Ac1, uint32_t Ad1
) 
{
    // Some constants like G_CCMUX_0 are 31 but stored in a 4 bits field...
    // It seems safe to ignore the last bit (treat 31 as 15)
    rdp.comb1Color.a = a0 & 0xF;
    rdp.comb1Color.b = b0 & 0xF;
    rdp.comb1Color.c = c0 & 0xF;
    rdp.comb1Color.d = d0 & 0xF;
    rdp.comb1Alpha.a = Aa0 & 0xF;
    rdp.comb1Alpha.b = Ab0 & 0xF;
    rdp.comb1Alpha.c = Ac0 & 0xF;
    rdp.comb1Alpha.d = Ad0 & 0xF;

    rdp.comb2Color.a = a1 & 0xF;
    rdp.comb2Color.b = b1 & 0xF;
    rdp.comb2Color.c = c1 & 0xF;
    rdp.comb2Color.d = d1 & 0xF;
    rdp.comb2Alpha.a = Aa1 & 0xF;
    rdp.comb2Alpha.b = Ab1 & 0xF;
    rdp.comb2Alpha.c = Ac1 & 0xF;
    rdp.comb2Alpha.d = Ad1 & 0xF;
}

void gfx_sp_set_other_mode(uint32_t shift, uint32_t num_bits, uint64_t mode) {
    uint64_t mask = (((uint64_t)1 << num_bits) - 1) << shift;
    uint64_t om = rdp.other_mode_l | ((uint64_t)rdp.other_mode_h << 32);
    om = (om & ~mask) | mode;
    rdp.other_mode_l = (uint32_t)om;
    rdp.other_mode_h = (uint32_t)(om >> 32);
}

void gfx_dp_set_tile(uint8_t fmt, uint32_t siz, uint32_t line, uint32_t tmem, uint8_t tile, uint32_t palette, uint32_t cmt, uint32_t maskt, uint32_t shiftt, uint32_t cms, uint32_t masks, uint32_t shifts) {
    rdp.tiles[tile].fmt = fmt;
    rdp.tiles[tile].siz = siz;
    rdp.tiles[tile].cms = cms;
    rdp.tiles[tile].cmt = cmt;
    rdp.tiles[tile].line_size_bytes = line * 8;
    rdp.tiles[tile].tmem_bytes = tmem * 8;

    //rdp.texture_to_load.tile_number = tmem / 256;
    rdp.texture_to_load.tile_number = 0;

    //print("gfx_dp_set_tile fmt %x siz %x line %x tmem %x tile %x palette %x cms %x cmt %x shifts %x shiftt %x\n", fmt, siz, line, tmem, tile, palette, cms, cmt, shifts, shiftt);
}

void gfx_dp_set_tile_size(uint8_t tile, uint16_t uls, uint16_t ult, uint16_t lrs, uint16_t lrt) {
    rdp.tiles[tile].uls = uls;
    rdp.tiles[tile].ult = ult;
    rdp.tiles[tile].lrs = lrs;
    rdp.tiles[tile].lrt = lrt;

    //print("gfx_dp_set_tile_size tile %x uls %x ult %x lrs %x lrt %x\n", tile, uls, ult, lrs, lrt);
}

void gfx_dp_set_texture_image(uint32_t format, uint32_t size, uint32_t width, void* addr) {
    rdp.texture_to_load.addr = addr;
    rdp.texture_to_load.siz = size;
    //print("- gfx_dp_set_texture_image addr: %016llx\n", rdp.texture_to_load.addr);
}

void gfx_normalize_vector(float v[3]) {
    float s = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= s;
    v[1] /= s;
    v[2] /= s;
}

void gfx_transposed_matrix_mul(float res[3], const float a[3], const float b[4][4]) {
    res[0] = a[0] * b[0][0] + a[1] * b[0][1] + a[2] * b[0][2];
    res[1] = a[0] * b[1][0] + a[1] * b[1][1] + a[2] * b[1][2];
    res[2] = a[0] * b[2][0] + a[1] * b[2][1] + a[2] * b[2][2];
}

void calculate_normal_dir(const Light_t *light, float coeffs[3]) {
    float light_dir[3] = {
        light->dir[0] / 127.0f,
        light->dir[1] / 127.0f,
        light->dir[2] / 127.0f
    };
    //gfx_transposed_matrix_mul(coeffs, light_dir, rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1]);
    gfx_transposed_matrix_mul(coeffs, light_dir, rsp.modelview_mtx[rsp.modelviewStackSize - 1].v);
    gfx_normalize_vector(coeffs);
}

void transform_vtx(float vec[4], struct RSPMtx* mtx)
{
    float x = vec[0] * mtx->v[0][0] + vec[1] * mtx->v[1][0] + vec[2] * mtx->v[2][0] + mtx->v[3][0];
    float y = vec[0] * mtx->v[0][1] + vec[1] * mtx->v[1][1] + vec[2] * mtx->v[2][1] + mtx->v[3][1];
    float z = vec[0] * mtx->v[0][2] + vec[1] * mtx->v[1][2] + vec[2] * mtx->v[2][2] + mtx->v[3][2];
    float w = vec[0] * mtx->v[0][3] + vec[1] * mtx->v[1][3] + vec[2] * mtx->v[2][3] + mtx->v[3][3];

    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
}

void normalize_vtx(float *v)
{
    float len;
    len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len > 0.0f)
    {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

void gfx_sp_vtx(const void* addr, u32 size, u32 count)
{
    //print("\ngfx_sp_vtx: count %x size: %x\n", count, size);

    struct gfxvtx* vtx = (struct gfxvtx*)addr;
    for (int i = 0; i < count; i++) {

        /*print("before vtx: %x x %d y %d z %d flags %x colour %x s %x t %x\n", i, 
            vtx[i].v[0], vtx[i].v[1], vtx[i].v[2],
            vtx[i].flags, vtx[i].colour, 
            vtx[i].s, vtx[i].t
        );*/

        {
            float vec[4] = { (float)vtx[i].v[0], (float)vtx[i].v[1], (float)vtx[i].v[2], 0.0f };
            transform_vtx(vec, &rsp.mp);
            
            rsp.vertices[i].x = vec[0];
            rsp.vertices[i].y = vec[1];
            rsp.vertices[i].z = vec[2];
            rsp.vertices[i].w = vec[3];
        }

        if (vtx[i].colour) {
            uint32_t color = rsp.colors[vtx[i].colour / 4];
            rsp.vertices[i].r = ((color >> 24) & 0xFF) / 255.0f;
            rsp.vertices[i].g = ((color >> 16) & 0xFF) / 255.0f;
            rsp.vertices[i].b = ((color >> 8 ) & 0xFF) / 255.0f;
            rsp.vertices[i].a = ((color      ) & 0xFF) / 255.0f;
        }

        rsp.vertices[i].u = FX_10_5_TO_F((uint32_t)vtx[i].s * rsp.tex_s >> 16);
        rsp.vertices[i].v = FX_10_5_TO_F((uint32_t)vtx[i].t * rsp.tex_t >> 16);

        if (rsp.geometry_mode & G_LIGHTING) 
        {
            if (rsp.geometry_mode & G_TEXTURE_GEN) 
            {
                uint32_t normals = rsp.colors[vtx[i].colour / 4];
                int8_t n0 = ((normals) & 0xFF);
                int8_t n1 = ((normals >> 8) & 0xFF);
                int8_t n2 = ((normals >> 16) & 0xFF);
                uint8_t alpha = ((normals >> 24) & 0xFF);

                float vec[4] = { (float)n0, (float)n1, (float)n2, 0.0f };
                //transform_vtx(vec, &rsp.mp);

                if (rsp.lookatEnable)
                {
                    vec[0] = 0.0f;
                    vec[1] = 0.0f;
                    vec[2] = 0.0f;
                }
                else
                {
                    transform_vtx(vec, &rsp.modelview_mtx[rsp.modelviewStackSize - 1]);

                    vec[2] = 0.0f;
                    normalize_vtx(vec);
                    
                    vec[0] = vec[0] * 128.0f;
                    vec[1] = vec[1] * 128.0f;
                    vec[2] = vec[2] * 128.0f;
                }
                
                float U = (0.5f * (vec[0] / 128.0f + 1.0f)) * (rsp.tex_s >> 6);
                float V = (0.5f * (vec[1] / 128.0f + 1.0f)) * (rsp.tex_t >> 6);

                rsp.vertices[i].u = U;
                rsp.vertices[i].v = V;
                rsp.vertices[i].r = 1.0f;
                rsp.vertices[i].g = 1.0f;
                rsp.vertices[i].b = 1.0f;
                rsp.vertices[i].a = 1.0f;
            }

            /*
            const Light_t lookat_x = {{0, 0, 0}, 0, {0, 0, 0}, 0, {127, 0, 0}, 0};
            const Light_t lookat_y = {{0, 0, 0}, 0, {0, 0, 0}, 0, {0, 127, 0}, 0};
            calculate_normal_dir(&lookat_x, rsp.current_lookat_coeffs[0]);
            calculate_normal_dir(&lookat_y, rsp.current_lookat_coeffs[1]);

            // n64devkit.square7.ch/pro-man/pro11/11-07.htm
            if (rsp.geometry_mode & G_TEXTURE_GEN) {
                //print("tx gen col index: %d\n", vtx[i].colour);
                if (rsp.geometry_mode & G_TEXTURE_GEN_LINEAR) {
                    print("tx G_TEXTURE_GEN_LINEAR\n");
                }

                uint32_t normals = rsp.colors[vtx[i].colour / 4];
                int8_t n0 = ((normals) & 0xFF);
                int8_t n1 = ((normals >> 8) & 0xFF);
                int8_t n2 = ((normals >> 16) & 0xFF);
                uint8_t alpha = ((normals >> 24) & 0xFF);

                uint32_t len = n0 * n0 + n1 * n1 + n2 * n2;

                //print("- n(%d, %d, %d) a %d len %d\n", n0, n1, n2, alpha, len);

                float dotx = 0, doty = 0;
                dotx += n0 * rsp.current_lookat_coeffs[0][0];
                dotx += n1 * rsp.current_lookat_coeffs[0][1];
                dotx += n2 * rsp.current_lookat_coeffs[0][2];
                doty += n0 * rsp.current_lookat_coeffs[1][0];
                doty += n1 * rsp.current_lookat_coeffs[1][1];
                doty += n2 * rsp.current_lookat_coeffs[1][2];

                //print("dotx: %f doty: %f\n", dotx, doty);
                //print("rsp.tex_s: %d rsp.tex_t: %d\n", rsp.tex_s, rsp.tex_t);


                //int32_t U = (int32_t)((dotx / 127.0f + 1.0f) / 4.0f * rsp.tex_s);
                //int32_t V = (int32_t)((doty / 127.0f + 1.0f) / 4.0f * rsp.tex_t);
                
                //float U = ((dotx / 127.0f + 1.0f)) * 32;
                //float V = ((doty / 127.0f + 1.0f)) * 32;

                float U = (0.5f * (dotx / 127.0f + 1.0f)) * (rsp.tex_s >> 6);
                float V = (0.5f * (doty / 127.0f + 1.0f)) * (rsp.tex_t >> 6);

                //print("u: %f\n", (dotx / 127.0f + 1.0f));
                print("u %f v %f\n", U, V);
                
                rsp.vertices[i].u = U;
                rsp.vertices[i].v = V;

                rsp.vertices[i].r = 1.0f;
                rsp.vertices[i].g = 1.0f;
                rsp.vertices[i].b = 1.0f;
                rsp.vertices[i].a = 1.0f;
            }

            */
        }
    }
}

void gfx_sp_set_color_array(void* ptr, u32 size)
{
    uint32_t* src = (uint32_t*)ptr;
    for (int i = 0; i < size / sizeof(uint32_t); i++)
    {
        rsp.colors[i] = src[i];
    }
}

void dbgRSPMtx(struct RSPMtx m, const char* title) {
    debugPrint(PC_DBG_FLAG_GFX, "mtx %s\n", title);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.v[0][0], m.v[0][1], m.v[0][2], m.v[0][3]);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.v[1][0], m.v[1][1], m.v[1][2], m.v[1][3]);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.v[2][0], m.v[2][1], m.v[2][2], m.v[2][3]);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.v[3][0], m.v[3][1], m.v[3][2], m.v[3][3]);
}

void dbgMtxf(Mtxf m, const char* title) {
    debugPrint(PC_DBG_FLAG_GFX, "mtx %s\n", title);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]);
    debugPrint(PC_DBG_FLAG_GFX, "%f %f %f %f\n", m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
}

void gfx_matrix_mul(float res[4][4], const float a[4][4], const float b[4][4]) {
    float tmp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[i][j] = a[i][0] * b[0][j] +
                        a[i][1] * b[1][j] +
                        a[i][2] * b[2][j] +
                        a[i][3] * b[3][j];
        }
    }
    memcpy(res, tmp, sizeof(tmp));
}

void gfx_mtx_copy1(Mtxf* src, struct RSPMtx* dst)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dst->v[i][j] = src->m[i][j];
        }
    }
}

void gfx_sp_mtx(const void* addr, u32 flags)
{
    // Note: the RSP structure stores matrices converted to floats, easier to manage
    if (flags & G_MTX_PROJECTION) 
    {
        // New matrices can be loaded onto the stack, multiplied with the top of the stack, but cannot be pushed or popped.
        if (flags & G_MTX_LOAD) 
        {
            Mtxf* mtx = (Mtxf*)addr;
            Mtxf tmp;
            mtxL2F(mtx, &tmp);

#if GFXPC_DBG_MATRICES
            dbgMtxf(tmp, "projection");
#endif
            gfx_mtx_copy1(&tmp, &rsp.projection_mtx);
        } 
        else 
        {
            debugPrint(PC_DBG_FLAG_GFX, "TODO handle gfx_sp_mtx G_MTX_PROJECTION / PUSH?\n");
            fatalExit();
        }
    } 
    else 
    {
        if (flags & G_MTX_LOAD) 
        {
            Mtxf* mtx = (Mtxf*)addr;
            if (mtx != NULL)
            {
                Mtxf tmp;
                mtxL2F(mtx, &tmp);

#if GFXPC_DBG_MATRICES
                dbgMtxf(tmp, "modelview");
#endif
                gfx_mtx_copy1(&tmp, &rsp.modelview_mtx[rsp.modelviewStackSize - 1]);
            }
            else
            {
                debugPrint(PC_DBG_FLAG_GFX, "[ERROR] gfx_sp_mtx: SPSEGMENT_MODEL_MTX was NULL!\n");
            }
        } 
       
        if (flags & G_MTX_PUSH)
        {
            // Unless this flag is present in a display list loaded from a file
            // This is not used by the game
            debugPrint(PC_DBG_FLAG_GFX, "TODO handle gfx_sp_mtx G_MTX_MODELVIEW / PUSH?\n");
            fatalExit();
        }
    }

    gfx_matrix_mul(rsp.mp.v, rsp.modelview_mtx[rsp.modelviewStackSize - 1].v, rsp.projection_mtx.v);
}

void gfx_sp_texture(u32 s, u32 t, u32 level, u32 tile, s32 on)
{
    if (on == G_ON)
    {
        rsp.tex_on = G_ON;
        rsp.tex_s = s;
        rsp.tex_t = t;
        rsp.tex_level = level;
        rsp.tex_tile = tile;
    }
    else
    {
        rsp.tex_on = G_OFF;
    }
}

void gfx_sp_set_geometry_mode(u32 clear, u32 mode) 
{
    rsp.geometry_mode &= ~clear;
    rsp.geometry_mode |= mode;
}

void gfx_sp_movemem(uint8_t index, uint8_t offset, const void* data)
{
    if (index == G_MV_LOOKATX || index == G_MV_LOOKATY) 
    {
        int lookatIndex = 0;
        Light_t* light = (Light_t*)data;

        if (index == G_MV_LOOKATX) {
            rsp.lookat_x = *light;
            lookatIndex = 0;
        } else if (index == G_MV_LOOKATY) {
            rsp.lookat_y = *light;
            lookatIndex = 1;
        }

        rsp.lookatEnable = (lookatIndex == 0) || (lookatIndex == 1 && (light->dir[0] != 0 || light->dir[1] != 0));
    } else {
        //print("unhandled movemev index %x\n", index);
    }

    /*
    switch (index) {
        case G_MV_VIEWPORT:
            gfx_calc_and_set_viewport((const Vp_t *) data);
            break;
#if 0
        case G_MV_LOOKATY:
        case G_MV_LOOKATX:
            memcpy(rsp.current_lookat + (index - G_MV_LOOKATY) / 2, data, sizeof(Light_t));
            //rsp.lights_changed = 1;
            break;
#endif
#ifdef F3DEX_GBI_2
        case G_MV_LIGHT: {
            int lightidx = offset / 24 - 2;
            if (lightidx >= 0 && lightidx <= MAX_LIGHTS) { // skip lookat
                // NOTE: reads out of bounds if it is an ambient light
                memcpy(rsp.current_lights + lightidx, data, sizeof(Light_t));
            }
            break;
        }
#else
        case G_MV_L0:
        case G_MV_L1:
        case G_MV_L2:
            // NOTE: reads out of bounds if it is an ambient light
            memcpy(rsp.current_lights + (index - G_MV_L0) / 2, data, sizeof(Light_t));
            break;
#endif
    }
    */
}

// https://ultra64.ca/files/documentation/online-manuals/man/pro-man/pro13/13-09.html#03
static void gfx_dp_load_block(uint8_t tile, uint32_t uls, uint32_t ult, uint32_t lrs, uint32_t dxt) {
    //char* modes[] = {"G_IM_SIZ_4b", "G_IM_SIZ_8b", "G_IM_SIZ_16b", "G_IM_SIZ_32b", "NULL", "G_IM_SIZ_DD"};
    //print("- gfx_dp_load_block tile %d uls %d ult %d lrs %d dxt %d\n", tile, uls, ult, lrs, dxt);
    //print("- load size: %s\n", modes[rdp.texture_to_load.siz]);

    // The lrs field rather seems to be number of pixels to load
    uint32_t word_size_shift;
    switch (rdp.texture_to_load.siz) {
        case G_IM_SIZ_4b:
            word_size_shift = 0; // Or -1? It's unused in SM64 anyway.
            break;
        case G_IM_SIZ_8b:
            word_size_shift = 0;
            break;
        case G_IM_SIZ_16b:
            word_size_shift = 1;
            break;
        case G_IM_SIZ_32b:
            word_size_shift = 2;
            break;
    }

    uint32_t size_bytes = (lrs + 1) << word_size_shift;

    // TODO: where the load block tile number is used?
    // This code works because dxt is not used and uls/ult are zero in the whole code base
    rdp.texture_load_info[rdp.texture_to_load.tile_number].size_bytes = size_bytes;
    rdp.texture_load_info[rdp.texture_to_load.tile_number].addr = rdp.texture_to_load.addr;
}

static void gfx_dp_load_tlut(uint8_t tile, uint32_t sh, uint32_t th, uint32_t sl, uint32_t tl) 
{
    /*
        For correctly handling the TLUT code, look at ares source code:
        void n64_rdp::cmd_load_tlut(uint64_t *cmd_buf)
    */
	const int32_t count = ((sh >> 2) - (sl >> 2) + 1) << 2;

    //print("gfx_dp_load_tlut tile %x sh %x th %x sl %x tl %x count %x\n", tile, sh, th, sl, tl, count);

    uint32_t width = rdp.tiles[tile].line_size_bytes;
	int32_t srcstart = ((tl >> 2) * (width << 1) + (sl >> 1)) >> 1;
    srcstart *= 4;

    //print(" srcstart %x\n", srcstart);

    // The palette is stored:
    // - directly at this address (then srcstart = 0)
    // - after the pixels (then srcstart > 0)
    rdp.palette = rdp.texture_to_load.addr + (srcstart * 2);
}

void gfx_start()
{
    glClearColor (0.0, 0.0, 0.0, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //print("Cached textures: %d\n", hashmap_count(textureCache.textures));
    rsp.modelviewStackSize = 1;
}

char* getOpcodeName(u8 opcode)
{
    char* out = "NONE";

    if (opcode == (u8)G_ENDDL) {
        out = "G_ENDDL";
    } else if (opcode == (u8)G_SETOTHERMODE_H) {
        out = "G_SETOTHERMODE_H";
    } else if (opcode == (u8)G_SETOTHERMODE_L) {
        out = "G_SETOTHERMODE_L";
    }  else if (opcode == (u8)G_DL) {
        out = "G_DL";
    } else if (opcode == (u8)G_FILLRECT) {
        out = "G_FILLRECT";
    } else if (opcode == (u8)G_TEXRECT) {
        out = "G_TEXRECT";
    } else if (opcode == (u8)G_SETFILLCOLOR) {
        out = "G_SETFILLCOLOR";
    } else if (opcode == (u8)G_SETPRIMCOLOR) {
        out = "G_SETPRIMCOLOR";
    } else if (opcode == (u8)G_SETCOMBINE) {
        out = "G_SETCOMBINE";
    } else if (opcode == (u8)G_SETTILE) {
        out = "G_SETTILE";
    } else if (opcode == (u8)G_SETTILESIZE) {
        out = "G_SETTILESIZE";
    } else if (opcode == (u8)G_SETTIMG) {
        out = "G_SETTIMG";
    } else if (opcode == (u8)G_LOADBLOCK) {
        out = "G_LOADBLOCK";
    } else if (opcode == (u8)G_LOADTLUT) {
        out = "G_LOADTLUT";
    } else if (opcode == (u8)G_SETSCISSOR) {
        out = "G_SETSCISSOR";
    } else if (opcode == (u8)G_RDPFULLSYNC) {
        out = "G_RDPFULLSYNC";
    } else if (opcode == (u8)G_RESERVED0) {
        out = "G_RESERVED0";
    } else if (opcode == (u8)G_SETCOLOR) {
        out = "G_SETCOLOR";
    } else if (opcode == (u8)G_RESERVED3) {
        out = "G_RESERVED3";
    } else if (opcode == (u8)G_SPNOOP) {
        out = "G_SPNOOP";
    } else if (opcode == (u8)G_NOOP) {
        out = "G_NOOP";
    } else if (opcode == (u8)G_SETCIMG) {
        out = "G_SETCIMG";
    } else if (opcode == (u8)G_RDPPIPESYNC) {
        out = "G_RDPPIPESYNC";
    } else if (opcode == (u8)G_MOVEMEM) {
        out = "G_MOVEMEM";
    } else if (opcode == (u8)G_MOVEWORD) {
        out = "G_MOVEWORD";
    } else if (opcode == (u8)G_MTX) {
        out = "G_MTX";
    } else if (opcode == (u8)G_RDPLOADSYNC) {
        out = "G_RDPLOADSYNC";
    } else if (opcode == (u8)G_VTX) {
        out = "G_VTX";
    } else if (opcode == (u8)G_SETFOGCOLOR) {
        out = "G_SETFOGCOLOR";
    } else if (opcode == (u8)G_CLEARGEOMETRYMODE) {
        out = "G_CLEARGEOMETRYMODE";
    } else if (opcode == (u8)G_TEXTURE) {
        out = "G_TEXTURE";
    } else if (opcode == (u8)G_SETGEOMETRYMODE) {
        out = "G_SETGEOMETRYMODE";
    } else if (opcode == (u8)G_TRI4) {
        out = "G_TRI4";
    } else if (opcode == (u8)G_RDPTILESYNC) {
        out = "G_RDPTILESYNC";
    } else if (opcode == (u8)G_SETZIMG) {
        out = "G_SETZIMG";
    } else if (opcode == (u8)G_SETENVCOLOR) {
        out = "G_SETENVCOLOR";
    }

    return out;
}

void debug_opcode(Gfx* gdl, int opcodeCount)
{
    u8 opcode = gdl->words.w0 >> 24;
    debugPrint(PC_DBG_FLAG_GFX, "- %x: opcode: %x (%s)\n", opcodeCount, opcode, getOpcodeName(opcode));
    debugPrint(PC_DBG_FLAG_GFX, "- cmd: @%llx : %016llx\n", gdl, *(uintptr_t*)gdl);

    struct cmd_extradata item = { gdl, NULL };
    struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
    if (ext) {
        debugPrint(PC_DBG_FLAG_GFX, "( %s : %d )\n", ext->file, ext->line);
    }
}

void gfx_execute_commands(Gfx* gdl, Gfx* gdlEnd)
{
#if GFXPC_DBG_COMMANDLIST
    debugPrint(PC_DBG_FLAG_GFX, "gfx_execute_commands:start() from %llx to %llx\n", gdl, gdlEnd);
#endif

#if GFXPC_DBG_EXTRADATA_MAP
    size_t iter = 0;
    void *item;
    while (hashmap_iter(gfxpcExtradataMap, &iter, &item)) {
        const struct cmd_extradata *ext = item;
        debugPrint(PC_DBG_FLAG_GFX, "extradata %llx % llx\n", ext->cmdPtr, ext->memPtr);
    }
#endif

#if GFXPC_DBG_WIREFRAME
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#endif

    u32 opcodeCount = 0;

    while (gdl < gdlEnd) 
    {
        u8 opcode = gdl->words.w0 >> 24;

#if GFXPC_DBG_COMMANDLIST
        debug_opcode(gdl, opcodeCount);
#endif

        if (opcode == (u8)G_ENDDL) 
        {
            // Display list processing stops here or if gdlEnd is reached
            // Most if not all display lists end with G_ENDDL
            break;
        } 
        else if (opcode == (u8)G_SETOTHERMODE_H) 
        {
            gfx_sp_set_other_mode(C0(8, 8) + 32, C0(0, 8), (uint64_t)gdl->words.w1 << 32);
        } 
        else if (opcode == (u8)G_SETOTHERMODE_L) 
        {
            gfx_sp_set_other_mode(C0(8, 8), C0(0, 8), gdl->words.w1);
        } 
        else if (opcode == (u8)G_DL) 
        {
            struct cmd_extradata item = { gdl, NULL };
            struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);

            if (C0(16, 1) == 0) 
            {
                if (ext) {
                    gfx_execute_commands((Gfx*)ext->memPtr, (Gfx*)0xFFFFFFFFFFFFFFFFULL);
                } else {
                    debugPrint(PC_DBG_FLAG_GFX, "- extradata not found\n");
                    fatalExit();
                }
            } 
            else 
            {
                if (ext) 
                {
                    gdl = ext->memPtr;
                    gdl--;
                } 
                else 
                {
                    debugPrint(PC_DBG_FLAG_GFX, "- extradata not found\n");
                    debugPrint(PC_DBG_FLAG_GFX, "0x%x G_DL (branch): %x\n", opcodeCount, (gdl->words.w1));
                    fatalExit();
                }
            }
        }
        else if (opcode == (u8)G_FILLRECT)
        {
            u32 lrx = C0(14, 10);
            u32 lry = C0(2, 10);
            u32 ulx = C1(14, 10);
            u32 uly = C1(2, 10);
            gfx_dp_fill_rectangle(ulx, uly, lrx, lry);
        } 
        else if (opcode == (u8)G_TEXRECT)
        {
            int32_t lrx, lry, tile, ulx, uly;
            uint32_t uls, ult, dsdx, dtdy;
            lrx = C0(12, 12);
            lry = C0(0, 12);
            tile = C1(24, 3);
            ulx = C1(12, 12);
            uly = C1(0, 12);
            ++gdl;
            uls = C1(16, 16);
            ult = C1(0, 16);
            ++gdl;
            dsdx = C1(16, 16);
            dtdy = C1(0, 16);

            gfx_dp_texture_rectangle(ulx, uly, lrx, lry, tile, uls, ult, dsdx, dtdy, opcode == G_TEXRECTFLIP);
        }
        else if (opcode == (u8)G_SETFILLCOLOR)
        {
            gfx_dp_set_fill_color(gdl->words.w1);
        }
        else if (opcode == (u8)G_SETPRIMCOLOR)
        {
            gfx_dp_set_prim_color(C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
        }
        else if (opcode == (u8)G_SETCOMBINE)
        {
            gfx_dp_set_combine_mode(
                C0(20, 4), C1(28, 4), C0(15, 5), C1(15, 3),
                C0(12, 3), C1(12, 3), C0(9, 3), C1(9, 3),
                C0(5, 4), C1(24, 4), C0(0, 5), C1(6, 3),
                C1(21, 3), C1(3, 3), C1(18, 3), C1(0, 3)
            );
        }
        else if (opcode == (u8)G_SETTILE)
        {
            gfx_dp_set_tile(C0(21, 3), C0(19, 2), C0(9, 9), C0(0, 9), C1(24, 3), C1(20, 4), C1(18, 2), C1(14, 4), C1(10, 4), C1(8, 2), C1(4, 4), C1(0, 4));
        }
        else if (opcode == (u8)G_SETTILESIZE)
        {
            gfx_dp_set_tile_size(C1(24, 3), C0(12, 12), C0(0, 12), C1(12, 12), C1(0, 12));
        }
        else if (opcode == (u8)G_SETTIMG)
        {
            struct cmd_extradata item = { gdl, NULL };
            struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
            if (ext) {
                gfx_dp_set_texture_image(C0(21, 3), C0(19, 2), C0(0, 10), ext->memPtr);
            } else {
                // No ext: we are probably in a display list loaded from a file
                u32 addr = gdl->words.w1;
                u32 segment = (addr & 0xF000000) >> 24;
                void* ptr = (void*)((uintptr_t)rsp.segments[segment]) + (uintptr_t)(addr & 0xFFFFFF);
                gfx_dp_set_texture_image(C0(21, 3), C0(19, 2), C0(0, 10), ptr);
            }
        }
        else if (opcode == (u8)G_LOADBLOCK)
        {
            gfx_dp_load_block(C1(24, 3), C0(12, 12), C0(0, 12), C1(12, 12), C1(0, 12));
        }
        else if (opcode == (u8)G_LOADTLUT)
        {
            u32 tile = C1(24, 3);
            u32 sh = C1(14, 10);
            u32 th = C1(2, 10);
            u32 sl = C0(14, 10);
            u32 tl = C0(2, 10);
            gfx_dp_load_tlut(tile, sh, th, sl, tl);
        }
        else if (opcode == (u8)G_SETSCISSOR)
        {
            // TODO
        }
        else if (opcode == (u8)G_RDPFULLSYNC) {}
        else if (opcode == (u8)G_RESERVED0) {}
        else if (opcode == (u8)G_SETCOLOR) 
        {
            u32 size = C0(16, 8) + 4;
            u32 addr = gdl->words.w1;

            struct cmd_extradata item = { gdl, NULL };
            struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
            if (ext) {
                gfx_sp_set_color_array(ext->memPtr, size);
            } else {
                u32 segment = (addr & 0xF000000) >> 24;
                void* ptr = (void*)((uintptr_t)rsp.segments[segment]) + (uintptr_t)(addr & 0xFFFFFF);

                gfx_sp_set_color_array(ptr, size);
            }

        }
        else if (opcode == (u8)G_RESERVED3) {}
        else if (opcode == (u8)G_SPNOOP) {}
        else if (opcode == (u8)G_NOOP) {}
        else if (opcode == (u8)G_SETCIMG)
        {
            // TODO
        }
        else if (opcode == (u8)G_RDPPIPESYNC) {}
        else if (opcode == (u8)G_MOVEMEM)
        {
            struct cmd_extradata item = { gdl, NULL };
            struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
            if (ext) {
                gfx_sp_movemem(C0(16, 8), 0, ext->memPtr);
            } else {
                debugPrint(PC_DBG_FLAG_GFX, "NO extradata for movemem\n");
            }

        }
        else if (opcode == (u8)G_MOVEWORD) 
        {
            u32 index = C0(0, 8);
            if (index == G_MW_SEGMENT) {
                u32 segment = C0(8, 16) >> 2;
                struct cmd_extradata item = { gdl, NULL };
                struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
                if (ext) 
                {
                    rsp.segments[segment] = ext->memPtr;
                }
            } else if (index == G_MW_CLIP) {
                // TODO
            } else if (index == G_MW_NUMLIGHT) {
                // TODO LIGHT
            } else if (index == G_MW_PERSPNORM) {
                rsp.perspnorm = gdl->words.w1;
            } else {
                debugPrint(PC_DBG_FLAG_GFX, "unhandled moveword: %x\n", index);
            }
        }
        else if (opcode == (u8)G_MTX)
        {
            u32 addr = gdl->words.w1;
            u32 flags = C0(16, 8);

            struct cmd_extradata item = { gdl, NULL };
            struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
            if (ext) {
                gfx_sp_mtx(ext->memPtr, flags);
            } else {
                // We are probably in a display list loaded from a file
                // Check the segment value
                u32 segment = (addr & 0xF000000) >> 24;
                if (segment == SPSEGMENT_MODEL_MTX) {
                    // Load the matrix found at this offset
                    Mtxf* mtx = (Mtxf*)((uintptr_t)rsp.segments[segment] + (uintptr_t)(addr & 0xffffff));
                    gfx_sp_mtx((void*)mtx, flags);
                }
            }
        }
        else if (opcode == (u8)G_RDPLOADSYNC) {}
        else if (opcode == (u8)G_VTX)
        {
            u32 size = C0(0, 16);
            u32 count = (C0(16, 8) >> 4) + 1;
            u32 addr = gdl->words.w1;

            struct cmd_extradata item = { gdl, NULL };
            struct cmd_extradata *ext = hashmap_get(gfxpcExtradataMap, &item);
            if (ext) {
                gfx_sp_vtx(ext->memPtr, size, count);
            } else {
                u32 segment = (addr & 0xF000000) >> 24;
                void* ptr = (void*)((uintptr_t)rsp.segments[segment]) + (uintptr_t)(addr & 0xFFFFFF);
                gfx_sp_vtx(ptr, size, count);
            }
        }
        else if (opcode == (u8)G_SETFOGCOLOR) 
        {
            // TODO
        }
        else if (opcode == (u8)G_CLEARGEOMETRYMODE) 
        {
            gfx_sp_set_geometry_mode(gdl->words.w1, 0);
        }
        else if (opcode == (u8)G_TEXTURE) 
        {
            u32 s = C1(16, 16);
            u32 t = C1(0, 16);
            u32 level = C0(11, 3);
            u32 tile = C0(8, 3);
            u32 on = C0(0, 8);

            gfx_sp_texture(s, t, level, tile, on);
        }
        else if (opcode == (u8)G_SETGEOMETRYMODE) 
        {
            u32 clear = C0(0, 24);
            u32 mode = gdl->words.w1;

            gfx_sp_set_geometry_mode(clear, mode);
        }
        else if (opcode == (u8)G_TRI4) 
        {
            uint8_t x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
            x1 = C1(0, 4);
            y1 = C1(4, 4);
            x2 = C1(8, 4);
            y2 = C1(12, 4);
            x3 = C1(16, 4);
            y3 = C1(20, 4);
            x4 = C1(24, 4);
            y4 = C1(28, 4);
            z1 = C0(0, 4);
            z2 = C0(4, 4);
            z3 = C0(8, 4);
            z4 = C0(12, 4);
            gfx_sp_tri4(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
        }
        else if (opcode == (u8)G_RDPTILESYNC) { } 
        else if (opcode == (u8)G_SETZIMG) { } 
        else if (opcode == (u8)G_SETENVCOLOR) {
            // TODO
        }
        else {
            debugPrint(PC_DBG_FLAG_GFX, "- %x: unhandled opcode: %x\n", opcodeCount, (u8)opcode);
            assert(false);
        }

        gdl++;
        opcodeCount++;
    }

    frames++;

#if GFXPC_DBG_COMMANDLIST
    debugPrint(PC_DBG_FLAG_GFX, "gfx_execute_commands:end() %d\n\n", opcodeCount);
#endif
}