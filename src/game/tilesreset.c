#include <ultra64.h>
#include "constants.h"
#include "game/tiles.h"
#include "game/bg.h"
#include "game/file.h"
#include "bss.h"
#include "data.h"
#include "types.h"

#include "print.h"
#include "byteswap.h"

void stageParseTiles(void);

void tilesReset(void)
{
	s32 index = stageGetIndex2(g_Vars.stagenum);

	if (index < 0) {
		index = 0;
	}

	g_LoadType = LOADTYPE_TILES;
	g_TileFileData.u8 = fileLoadToNew(g_Stages[index].tilefileid, FILELOADMETHOD_DEFAULT);
	g_TileNumRooms = swap_uint32(*g_TileFileData.u32);
	g_TileRooms = g_TileFileData.u32 + 1;

	/*
		tile file structure
		- num rooms (g_TileNumRooms): u32
		- offsets (g_TileRooms): u32 * g_TileNumRooms
		- rooms?
	*/

	// PC: convert endianess
	for (u32 i = 0; i <= g_TileNumRooms; i++) {
		g_TileRooms[i] = swap_uint32(g_TileRooms[i]);
	}

	print("tilesReset()\n");
	print("- g_TileNumRooms: %x\n", g_TileNumRooms);

	stageParseTiles();
}

#define mult6(a) (((a) << 1) + ((a) << 2))

void stageParseTiles(void)
{
	// Note PC:
	// Not sure this work, compare with N64 trace
	// g_TileNumRooms is 0x10f but the loop goes over way more items than that
	struct geo *geo = (struct geo *)(g_TileFileData.u8 + g_TileRooms[0]);
	struct geo *end = (struct geo *)(g_TileFileData.u8 + g_TileRooms[g_TileNumRooms]);

	//print("g_TileRooms[0]: %x\n", g_TileRooms[0]);
	//print("g_TileRooms[g_TileNumRooms]: %x\n", g_TileRooms[g_TileNumRooms]);

	u32 count = 0;
	while (geo < end) {
		// PC: convert endianess
		geo->flags = swap_uint16(geo->flags);
		//print("count: %d/%d - geo type: %x\n", count, g_TileNumRooms, geo->type);
		
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;

			tile->xmin = mult6(tile->xmin) + 14;
			tile->xmax = mult6(tile->xmax) + 14;
			tile->ymin = mult6(tile->ymin) + 16;
			tile->ymax = mult6(tile->ymax) + 16;
			tile->zmin = mult6(tile->zmin) + 18;
			tile->zmax = mult6(tile->zmax) + 18;

			tile->floortype = swap_uint16(tile->floortype);
			tile->floorcol = swap_uint16(tile->floorcol);

			for (u32 i = 0; i < geo->numvertices; i++) {
				tile->vertices[i][0] = swap_int16(tile->vertices[i][0]);
				tile->vertices[i][1] = swap_int16(tile->vertices[i][1]);
				tile->vertices[i][2] = swap_int16(tile->vertices[i][2]);
			}

			geo = (struct geo *)((u8 *)geo + (uintptr_t)(geo->numvertices - 0x40) * 6 + 0x18e);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;

			tile->floortype = swap_uint16(tile->floortype);
			tile->floorcol = swap_uint16(tile->floorcol);
			
			for (u32 i = 0; i < geo->numvertices; i++) {
				tile->vertices[i].f[0] = swap_f32(tile->vertices[i].f[0]);
				tile->vertices[i].f[1] = swap_f32(tile->vertices[i].f[1]);
				tile->vertices[i].f[2] = swap_f32(tile->vertices[i].f[2]);
			}

			geo = (struct geo *)((u8 *)geo + (uintptr_t)(geo->numvertices - 0x40) * 12 + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct geoblock *tile = (struct geoblock *) geo;

			tile->ymax = swap_f32(tile->ymax);
			tile->ymin = swap_f32(tile->ymin);

			for (u32 i = 0; i < geo->numvertices; i++) {
				tile->vertices[i][0] = swap_f32(tile->vertices[i][0]);
				tile->vertices[i][1] = swap_f32(tile->vertices[i][1]);
			}

			geo = (struct geo *)((u8 *)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *tile = (struct geocyl *) geo;

			tile->ymax = swap_f32(tile->ymax);
			tile->ymin = swap_f32(tile->ymin);
			tile->x = swap_f32(tile->x);
			tile->z = swap_f32(tile->z);
			tile->radius = swap_f32(tile->radius);

			geo = (struct geo *)((u8 *)geo + sizeof(struct geocyl));
		}

		count++;
	}
}
