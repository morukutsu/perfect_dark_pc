const fs = require("fs");

// PDARK_LIB_FILES
console.log("PDARK_LIB_FILES\n\n");
{
  const dir = fs.readdirSync("../../src/lib");
  const prefix = "${PDARK_SRC_DIR}/lib/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}

{
  const dir = fs.readdirSync("../../src/lib/naudio");
  const prefix = "${PDARK_SRC_DIR}/lib/naudio/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}

// PDARK_GAME_FILES
console.log("PDARK_GAME_FILES\n\n");
{
  const dir = fs.readdirSync("../../src/game");
  const prefix = "${PDARK_SRC_DIR}/game/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}

{
  const dir = fs.readdirSync("../../src/game/modeldata");
  const prefix = "${PDARK_SRC_DIR}/game/modeldata/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}

{
  const dir = fs.readdirSync("../../src/game/mplayer");
  const prefix = "${PDARK_SRC_DIR}/game/mplayer/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}

{
  const dir = fs.readdirSync("../../src/game/stubs");
  const prefix = "${PDARK_SRC_DIR}/game/stubs/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}

// PDARK_LIB_ULTRA_FILES
console.log("PDARK_LIB_ULTRA_FILES\n\n");
{
  const dir = fs.readdirSync("../../src/lib/ultra/gu");
  const prefix = "${PDARK_SRC_DIR}/lib/ultra/gu/";
  for (const e of dir) {
    if (e.endsWith(".c")) console.log('"' + prefix + e + '"');
  }
}
