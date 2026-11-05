#pragma once
#include <obs-module.h>

#include <util/base.h>
#include <util/dstr.h>
#include <util/darray.h>
#include <util/platform.h>
#include <util/threading.h>

#include <stdio.h>

extern gs_texrender_t *create_or_reset_texrender(gs_texrender_t *render);
extern gs_texrender_t *create_or_reset_texrender_high(gs_texrender_t *render);
extern gs_texrender_t *
create_or_reset_texrender_format(gs_texrender_t *render,
				 enum gs_color_format format);
extern void set_blending_parameters();
extern bool add_source_to_list(void *data, obs_source_t *source);
gs_effect_t *load_shader_effect(gs_effect_t *effect,
				const char *effect_file_path);
extern char *load_shader_from_file(const char *file_name);
extern void setting_visibility(const char *prop_name, bool visible,
			       obs_properties_t *props);
extern void texrender_set_texture(gs_texture_t *source, gs_texrender_t *dest);

// A source referenced by name from the settings (a fill source, or the input
// of a stroke/glow/shadow source).
//
// The settings only change the name: source_ref_set_name() may be called
// from any thread. The weak reference is resolved and used only on the
// graphics thread (source_ref_tick() from video_tick, source_ref_get() from
// video_render), so it is never released while the render code is using it.
struct source_ref {
	pthread_mutex_t mutex;
	struct dstr name; // protected by mutex
	bool name_changed; // protected by mutex

	// Graphics thread only.
	struct dstr resolved_name;
	obs_weak_source_t *weak;
	float retry_timer;
};

extern void source_ref_init(struct source_ref *ref);
extern void source_ref_free(struct source_ref *ref);
extern void source_ref_set_name(struct source_ref *ref, const char *name);
extern void source_ref_tick(struct source_ref *ref, float seconds);
// Returns a new strong reference (release it), or NULL.
extern obs_source_t *source_ref_get(struct source_ref *ref);
extern bool source_ref_has_name(struct source_ref *ref);

// Renders a source into *texrender (created, or recreated if the source's
// color format changed, and reused between frames) and returns its texture,
// or NULL if the source has no size.
extern gs_texture_t *render_source_to_texture(obs_source_t *source,
					      gs_texrender_t **texrender);

// True if rendering `source` from inside `context` (a filter or a source
// that draws another source) would just render `context` again, e.g. when a
// source is picked as its own fill source.
extern bool is_self_reference(obs_source_t *context, obs_source_t *source);
