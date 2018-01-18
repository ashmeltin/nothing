#include <SDL2/SDL.h>

#include <math.h>
#include <assert.h>

#include "./camera.h"
#include "./error.h"

#define RATIO_X 16.0f
#define RATIO_Y 9.0f

struct camera_t {
    int debug_mode;
    point_t position;
};

camera_t *create_camera(point_t position)
{
    camera_t *camera = malloc(sizeof(camera_t));

    if (camera == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    camera->position = position;
    camera->debug_mode = 0;

    return camera;
}

void destroy_camera(camera_t *camera)
{
    assert(camera);

    free(camera);
}

static vec_t effective_ratio(const SDL_Rect *view_port)
{
    if ((float) view_port->w / RATIO_X > (float) view_port->h / RATIO_Y) {
        return vec(RATIO_X, (float) view_port->h / ((float) view_port->w / RATIO_X));
    } else {
        return vec((float) view_port->w / ((float) view_port->h / RATIO_Y), RATIO_Y);
    }
}

static vec_t effective_scale(const SDL_Rect *view_port)
{
    return vec_entry_div(
        vec((float) view_port->w, (float) view_port->h),
        vec_scala_mult(effective_ratio(view_port), 50.0f));
}

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const rect_t *rect)
{
    assert(camera);
    assert(render);
    assert(rect);

    SDL_Rect view_port;

    SDL_RenderGetViewport(render, &view_port);

    const vec_t scale = effective_scale(&view_port);
    const SDL_Rect sdl_rect = {
        .x = (int) roundf((rect->x - camera->position.x) * scale.x + (float) view_port.w * 0.5f),
        .y = (int) roundf((rect->y - camera->position.y) * scale.y + (float) view_port.h * 0.5f),
        .w = (int) roundf(rect->w * scale.x),
        .h = (int) roundf(rect->h * scale.y)
    };

    if (camera->debug_mode) {
        if (SDL_RenderDrawRect(render, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    } else {
        if (SDL_RenderFillRect(render, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    }

    return 0;
}

void camera_center_at(camera_t *camera, point_t position)
{
    assert(camera);
    camera->position = position;
}

void camera_toggle_debug_mode(camera_t *camera)
{
    assert(camera);
    camera->debug_mode = !camera->debug_mode;
}
