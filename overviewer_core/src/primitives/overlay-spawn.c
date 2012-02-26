/*
 * This file is part of the Minecraft Overviewer.
 *
 * Minecraft Overviewer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * Minecraft Overviewer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the Overviewer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "overlay.h"
#include <math.h>

typedef struct {
    /* inherits from overlay */
    RenderPrimitiveOverlay parent;
    
    PyObject *skylight, *blocklight;
} RenderPrimitiveSpawn;

static void get_color(void *data, RenderState *state,
                      unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a) {
    
    RenderPrimitiveSpawn* self = (RenderPrimitiveSpawn *)data;
    int x = state->x, y = state->y, z = state->z;
    int z_light = z + 1;
    unsigned char blocklight, skylight;
    PyObject *block_py;
    
    /* set a nice, pretty red color */
    *r = 229;
    *g = 36;
    *b = 38;
    
    /* default to no overlay, until told otherwise */
    *a = 0;
    
    if (block_has_property(state->block, NOSPAWN)) {
        /* nothing can spawn on this */
        return;
    }
    
    blocklight = getArrayByte3D(self->blocklight, x, y, MIN(127, z_light));
    
    /* if we're at the top, force 15 (brightest!) skylight */
    if (z_light == 128) {
        skylight = 15;
    } else {
        skylight = getArrayByte3D(self->skylight, x, y, z_light);
    }
    
    if (MAX(blocklight, skylight) <= 7) {
        /* hostile mobs spawn in daylight */
        *a = 240;
    } else if (MAX(blocklight, skylight - 11) <= 7) {
        /* hostile mobs spawn at night */
        *a = 150;
    }
}

static int
overlay_spawn_start(void *data, RenderState *state, PyObject *support) {
    RenderPrimitiveSpawn* self;

    /* first, chain up */
    int ret = primitive_overlay.start(data, state, support);
    if (ret != 0)
        return ret;
    
    /* now do custom initializations */
    self = (RenderPrimitiveSpawn *)data;
    self->blocklight = get_chunk_data(state, CURRENT, BLOCKLIGHT, 1);
    self->skylight = get_chunk_data(state, CURRENT, SKYLIGHT, 1);
    
    /* setup custom color */
    self->parent.get_color = get_color;
    
    return 0;
}

static void
overlay_spawn_finish(void *data, RenderState *state) {
    /* first free all *our* stuff */
    RenderPrimitiveSpawn* self = (RenderPrimitiveSpawn *)data;
    
    Py_DECREF(self->blocklight);
    Py_DECREF(self->skylight);
    
    /* now, chain up */
    primitive_overlay.finish(data, state);
}

RenderPrimitiveInterface primitive_overlay_spawn = {
    "overlay-spawn",
    sizeof(RenderPrimitiveSpawn),
    overlay_spawn_start,
    overlay_spawn_finish,
    NULL,
    NULL,
    overlay_draw,
};
