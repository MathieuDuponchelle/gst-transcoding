#include <json-glib/json-glib.h>
#include "job.h"

G_DEFINE_QUARK (application/unknown, gst_transcoding_format_none)

G_DEFINE_QUARK (video/x-matroska, gst_transcoding_format_matroska)

G_DEFINE_QUARK (video/x-h264, gst_transcoding_format_h264)

G_DEFINE_QUARK (audio/x-aac, gst_transcoding_format_aac)

typedef enum
{
  AUDIO,
  VIDEO,
} MediaType;

typedef struct
{
  GstTranscodingInput *input;
  GstTranscodingOutput *output;
  GstTranscodingFormat format;
} GstTranscodingStreamProfilePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GstTranscodingStreamProfile, gst_transcoding_stream_profile, G_TYPE_OBJECT)

struct _GstTranscodingVideoProfile
{
  GstTranscodingStreamProfile parent;
};

G_DEFINE_TYPE (GstTranscodingVideoProfile, gst_transcoding_video_profile, GST_TRANSCODING_TYPE_STREAM_PROFILE)

struct _GstTranscodingAudioProfile
{
  GstTranscodingStreamProfile parent;
};

G_DEFINE_TYPE (GstTranscodingAudioProfile, gst_transcoding_audio_profile, GST_TRANSCODING_TYPE_STREAM_PROFILE)

struct _GstTranscodingContainerProfile
{
  GObject parent;
  GstTranscodingAudioProfile *meta_audio_profile;
  GstTranscodingVideoProfile *meta_video_profile;
  GstTranscodingFormat format;
};

G_DEFINE_TYPE (GstTranscodingContainerProfile, gst_transcoding_container_profile, G_TYPE_OBJECT)

struct _GstTranscodingInput
{
  GObject parent;
  gchar *uri;
  GHashTable *profiles;
  gboolean auto_link;
};

G_DEFINE_TYPE (GstTranscodingInput, gst_transcoding_input, G_TYPE_OBJECT)

struct _GstTranscodingOutput
{
  GObject parent;
  gchar *uri;
  gboolean auto_link;
  GstTranscodingContainerProfile *profile;
};

G_DEFINE_TYPE (GstTranscodingOutput, gst_transcoding_output, G_TYPE_OBJECT)

struct _GstTranscodingJob
{
  GObject parent;

  GHashTable *inputs;
  GHashTable *outputs;
};

G_DEFINE_TYPE (GstTranscodingJob, gst_transcoding_job, G_TYPE_OBJECT)

static void
gst_transcoding_stream_profile_class_init (GstTranscodingStreamProfileClass *klass)
{
}

static void
gst_transcoding_stream_profile_init (GstTranscodingStreamProfile *self)
{
}

static void
gst_transcoding_video_profile_class_init (GstTranscodingVideoProfileClass *klass)
{
}

static void
gst_transcoding_video_profile_init (GstTranscodingVideoProfile *self)
{
}

static void
gst_transcoding_audio_profile_class_init (GstTranscodingAudioProfileClass *klass)
{
}

static void
gst_transcoding_audio_profile_init (GstTranscodingAudioProfile *self)
{
}

static void
gst_transcoding_container_profile_class_init (GstTranscodingContainerProfileClass *klass)
{
}

static void
gst_transcoding_container_profile_init (GstTranscodingContainerProfile *self)
{
}

static void
gst_transcoding_input_class_init (GstTranscodingInputClass *klass)
{
}

static void
gst_transcoding_input_init (GstTranscodingInput *self)
{
}

static void
gst_transcoding_output_class_init (GstTranscodingOutputClass *klass)
{
}

static void
gst_transcoding_output_init (GstTranscodingOutput *self)
{
}

static void
job_finalize (GObject *object)
{
  GstTranscodingJob *self = GST_TRANSCODING_JOB (object);

  g_hash_table_unref (self->inputs);
  g_hash_table_unref (self->outputs);

  G_OBJECT_CLASS (gst_transcoding_job_parent_class)->finalize (object);
}

static void
gst_transcoding_job_class_init (GstTranscodingJobClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = job_finalize;
}

static void
gst_transcoding_job_init (GstTranscodingJob *self)
{
  self->inputs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->outputs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}

GstTranscodingJob *
gst_transcoding_job_new (void)
{
  return g_object_new (GST_TRANSCODING_TYPE_JOB, NULL);
}

static GstTranscodingInput *
job_create_input (GstTranscodingJob *self, const gchar *uri)
{
  GstTranscodingInput *ret = g_object_new(GST_TRANSCODING_TYPE_INPUT, NULL);

  ret->uri = g_strdup (uri);
  ret->profiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);

  g_hash_table_insert (self->inputs, (gpointer) g_strdup (uri), g_object_ref (ret));

  ret->auto_link = FALSE;

  return ret;
}

static GstTranscodingInput *
job_get_input (GstTranscodingJob *self, const gchar *uri, gboolean create)
{
  GstTranscodingInput *ret = g_hash_table_lookup (self->inputs, uri);

  if (!ret && create)
    ret = job_create_input (self, uri);

  return ret;
}

static GstTranscodingContainerProfile *
job_create_container_profile_from_extension (const gchar *uri)
{
  GstTranscodingContainerProfile *ret = gst_transcoding_container_profile_new (NULL, NULL);

  /* TODO: add some more :) */
  if (g_str_has_suffix(uri, ".mkv")) {
    ret->format = GST_TRANSCODING_FORMAT_MATROSKA;
  } else {
    ret->format = GST_TRANSCODING_FORMAT_NONE;
  }

  return ret;
}

static GstTranscodingOutput *
job_create_output (GstTranscodingJob *self, GstTranscodingContainerProfile *profile, const gchar *uri)
{
  GstTranscodingOutput *ret = g_object_new(GST_TRANSCODING_TYPE_OUTPUT, NULL);

  ret->uri = g_strdup (uri);

  if (profile)
    ret->profile = profile;
  else
    ret->profile = job_create_container_profile_from_extension (uri);

  g_hash_table_insert (self->outputs, (gpointer) g_strdup (uri), g_object_ref (ret));

  ret->auto_link = FALSE;

  return ret;
}

static GstTranscodingOutput *
job_get_output (GstTranscodingJob *self, GstTranscodingContainerProfile *profile, const gchar *uri, gboolean create)
{
  GstTranscodingOutput *ret = g_hash_table_lookup (self->outputs, uri);

  if (!ret && create)
    ret = job_create_output (self, profile, uri);

  return ret;
}

static void
video_profile_copy_into (GstTranscodingVideoProfile *src, GstTranscodingVideoProfile *dst)
{
  GstTranscodingStreamProfilePrivate *srcpriv =
    gst_transcoding_stream_profile_get_instance_private ((GstTranscodingStreamProfile *) src);
  GstTranscodingStreamProfilePrivate *dstpriv =
    gst_transcoding_stream_profile_get_instance_private ((GstTranscodingStreamProfile *) dst);

  dstpriv->format = srcpriv->format;

  /* TODO: copy video-specific attributes */
}

static void
audio_profile_copy_into (GstTranscodingAudioProfile *src, GstTranscodingAudioProfile *dst)
{
  GstTranscodingStreamProfilePrivate *srcpriv =
    gst_transcoding_stream_profile_get_instance_private ((GstTranscodingStreamProfile *) src);
  GstTranscodingStreamProfilePrivate *dstpriv =
    gst_transcoding_stream_profile_get_instance_private ((GstTranscodingStreamProfile *) dst);

  dstpriv->format = srcpriv->format;

  /* TODO: copy audio-specific attributes */
}

static GstTranscodingStreamProfile *
job_map_stream(GstTranscodingJob *self,
    const gchar *in_uri, const gchar *stream_id, MediaType media_type, const gchar *out_uri)
{
  GstTranscodingInput *input = job_get_input (self, in_uri, TRUE);
  GstTranscodingOutput *output = job_get_output (self, NULL, out_uri, TRUE);
  GstTranscodingStreamProfile *ret;
  GstTranscodingStreamProfilePrivate *priv;
  GPtrArray *profiles;

  if (media_type == VIDEO) {
    ret = (GstTranscodingStreamProfile *) gst_transcoding_video_profile_new ();
  } else {
    ret = (GstTranscodingStreamProfile *) gst_transcoding_audio_profile_new ();
  }

  priv = gst_transcoding_stream_profile_get_instance_private (ret);
  priv->input = input;
  priv->output = output;

  if (media_type == VIDEO) {
    video_profile_copy_into (output->profile->meta_video_profile, (GstTranscodingVideoProfile *) ret);
  } else {
    audio_profile_copy_into (output->profile->meta_audio_profile, (GstTranscodingAudioProfile *) ret);
  }

  if (!g_hash_table_contains (input->profiles, stream_id)) {
    profiles = g_ptr_array_new_with_free_func(g_object_unref);
    g_ptr_array_add (profiles, g_object_ref (ret));
    g_hash_table_insert (input->profiles, g_strdup (stream_id), profiles);
  } else {
    GstTranscodingStreamProfile *prior_profile;

    profiles = g_hash_table_lookup (input->profiles, stream_id);
    prior_profile = (GstTranscodingStreamProfile *) g_ptr_array_index (profiles, 0);

    /* Detect impossible situation */
    if (G_OBJECT_TYPE (prior_profile) != G_OBJECT_TYPE (ret)) {
      g_object_unref (ret);
      ret = NULL;
    } else {
      g_ptr_array_add (profiles, g_object_ref (ret));
    }
  }

  return ret;
}

GstTranscodingVideoProfile *
gst_transcoding_job_map_video_stream (GstTranscodingJob *self, const gchar *in_uri, const gchar *stream_id, const gchar *out_uri)
{
  return (GstTranscodingVideoProfile *) job_map_stream (self, in_uri, stream_id, VIDEO, out_uri);
}

GstTranscodingAudioProfile *
gst_transcoding_job_map_audio_stream (GstTranscodingJob *self, const gchar *in_uri, const gchar *stream_id, const gchar *out_uri)
{
  return (GstTranscodingAudioProfile *) job_map_stream (self, in_uri, stream_id, AUDIO, out_uri);
}

GstTranscodingInput * gst_transcoding_job_add_input (GstTranscodingJob *self,
                                                     const gchar *uri)
{
  GstTranscodingInput *ret;

  if (job_get_input (self, uri, FALSE))
    return NULL;

  ret = job_create_input (self, uri);

  ret->auto_link = TRUE;

  return ret;
}

GstTranscodingOutput * gst_transcoding_job_add_output (GstTranscodingJob *self,
                                                       const gchar *uri,
                                                       GstTranscodingContainerProfile *profile)
{
  GstTranscodingOutput *ret;

  if (job_get_output (self, NULL, uri, FALSE))
    return NULL;

  ret = job_create_output (self, profile, uri);

  ret->auto_link = TRUE;

  return ret;
}

static void
audio_profile_to_json (GstTranscodingAudioProfile *profile, JsonBuilder *builder)
{
  /* TODO: serialize audio-specific properties */
}

static void
video_profile_to_json (GstTranscodingVideoProfile *profile, JsonBuilder *builder)
{
  /* TODO: serialize video-specific properties */
}

static void
profile_to_json (GstTranscodingStreamProfile *profile, JsonBuilder *builder)
{
  GstTranscodingStreamProfilePrivate *priv = gst_transcoding_stream_profile_get_instance_private (profile);

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "format");
  json_builder_add_string_value (builder, g_quark_to_string (priv->format));

  if (GST_TRANSCODING_IS_AUDIO_PROFILE (profile)) {
    audio_profile_to_json ((GstTranscodingAudioProfile *) profile, builder);
  } else {
    video_profile_to_json ((GstTranscodingVideoProfile *) profile, builder);
  }

  if (priv->output) {
    json_builder_set_member_name (builder, "output");
    json_builder_add_string_value (builder, priv->output->uri);
  }

  json_builder_end_object (builder);
}

static void
container_profile_to_json (GstTranscodingContainerProfile *profile, JsonBuilder *builder)
{
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "format");
  json_builder_add_string_value (builder, g_quark_to_string (profile->format));
  json_builder_set_member_name (builder, "meta-audio-profile");
  profile_to_json ((GstTranscodingStreamProfile *) profile->meta_audio_profile, builder);
  json_builder_set_member_name (builder, "meta-video-profile");
  profile_to_json ((GstTranscodingStreamProfile *) profile->meta_video_profile, builder);
  json_builder_end_object (builder);
}

static void
output_to_json (gchar *uri, GstTranscodingOutput *output, JsonBuilder *builder)
{
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "uri");
  json_builder_add_string_value (builder, uri);
  json_builder_set_member_name (builder, "autolink");
  json_builder_add_boolean_value (builder, output->auto_link);
  json_builder_set_member_name (builder, "container-profile");
  container_profile_to_json (output->profile, builder);
  json_builder_end_object (builder);
}

static void
streams_to_json (gchar *stream_id, GPtrArray *profiles, JsonBuilder *builder)
{
  GstTranscodingStreamProfile *first_profile;

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "stream-id");
  json_builder_add_string_value (builder, stream_id);
  first_profile = g_ptr_array_index (profiles, 0);
  json_builder_set_member_name (builder, "media-type");
  if (GST_TRANSCODING_IS_AUDIO_PROFILE (first_profile))
    json_builder_add_string_value (builder, "audio");
  else
    json_builder_add_string_value (builder, "video");
  json_builder_set_member_name (builder, "profiles");
  json_builder_begin_array (builder);
  g_ptr_array_foreach (profiles, (GFunc) profile_to_json, builder);
  json_builder_end_array(builder);
  json_builder_end_object (builder);
}

static void
input_to_json (gchar *uri, GstTranscodingInput *input, JsonBuilder *builder)
{
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "uri");
  json_builder_add_string_value (builder, uri);
  json_builder_set_member_name (builder, "autolink");
  json_builder_add_boolean_value (builder, input->auto_link);
  json_builder_set_member_name (builder, "streams");
  json_builder_begin_array (builder);
  g_hash_table_foreach (input->profiles, (GHFunc) streams_to_json, builder);
  json_builder_end_array (builder);
  json_builder_end_object (builder);
}

gchar *
gst_transcoding_job_to_json (GstTranscodingJob *self, gboolean pretty)
{
  gchar *ret;
  JsonBuilder *builder = json_builder_new();
  JsonNode *root;

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "outputs");

  json_builder_begin_array (builder);
  g_hash_table_foreach (self->outputs, (GHFunc) output_to_json, builder);
  json_builder_end_array (builder);

  json_builder_set_member_name (builder, "inputs");
  json_builder_begin_array (builder);
  g_hash_table_foreach (self->inputs, (GHFunc) input_to_json, builder);
  json_builder_end_array (builder);

  json_builder_end_object (builder);

  root = json_builder_get_root (builder);
  ret = json_to_string (root, TRUE);

  json_node_unref (root);
  g_object_unref (builder);

  return ret;
}

GstTranscodingInput *
gst_transcoding_stream_profile_get_input (GstTranscodingStreamProfile *self)
{
  GstTranscodingStreamProfilePrivate *priv = gst_transcoding_stream_profile_get_instance_private (self);

  return priv->input ? g_object_ref (priv->input) : NULL;
}

GstTranscodingOutput *
gst_transcoding_stream_profile_get_output (GstTranscodingStreamProfile *self)
{
  GstTranscodingStreamProfilePrivate *priv = gst_transcoding_stream_profile_get_instance_private (self);

  return priv->output ? g_object_ref (priv->output) : NULL;
}

GstTranscodingFormat
gst_transcoding_stream_profile_get_format (GstTranscodingStreamProfile *self)
{
  GstTranscodingStreamProfilePrivate *priv = gst_transcoding_stream_profile_get_instance_private (self);

  return priv->format;
}

void
gst_transcoding_stream_profile_set_format (GstTranscodingStreamProfile *self, GstTranscodingFormat format)
{
  GstTranscodingStreamProfilePrivate *priv = gst_transcoding_stream_profile_get_instance_private (self);

  priv->format = format;
}

GstTranscodingVideoProfile *
gst_transcoding_video_profile_new (void)
{
  GstTranscodingVideoProfile *ret = g_object_new (GST_TRANSCODING_TYPE_VIDEO_PROFILE, NULL);
  GstTranscodingStreamProfilePrivate *priv =
    gst_transcoding_stream_profile_get_instance_private ((GstTranscodingStreamProfile *) ret);

  priv->format = GST_TRANSCODING_FORMAT_NONE;
  return ret;
}

GstTranscodingAudioProfile *
gst_transcoding_audio_profile_new (void)
{
  GstTranscodingAudioProfile *ret = g_object_new (GST_TRANSCODING_TYPE_AUDIO_PROFILE, NULL);
  GstTranscodingStreamProfilePrivate *priv =
    gst_transcoding_stream_profile_get_instance_private ((GstTranscodingStreamProfile *) ret);

  priv->format = GST_TRANSCODING_FORMAT_NONE;
  return ret;
}

GstTranscodingFormat
gst_transcoding_container_profile_get_format (GstTranscodingContainerProfile *self)
{
  return self->format;
}

void
gst_transcoding_container_profile_set_format (GstTranscodingContainerProfile *self, GstTranscodingFormat format)
{
  self->format = format;
}

GstTranscodingVideoProfile *
gst_transcoding_container_profile_get_meta_video_profile (GstTranscodingContainerProfile * self)
{
  return g_object_ref (self->meta_video_profile);
}

GstTranscodingAudioProfile *
gst_transcoding_container_profile_get_meta_audio_profile (GstTranscodingContainerProfile * self)
{
  return g_object_ref (self->meta_audio_profile);
}

GstTranscodingContainerProfile * gst_transcoding_container_profile_new (GstTranscodingAudioProfile *meta_audio_profile,
    GstTranscodingVideoProfile *meta_video_profile)
{
  GstTranscodingContainerProfile *ret = g_object_new (GST_TRANSCODING_TYPE_CONTAINER_PROFILE, NULL);

  ret->meta_video_profile = meta_video_profile ? meta_video_profile : gst_transcoding_video_profile_new ();
  ret->meta_audio_profile = meta_audio_profile ? meta_audio_profile : gst_transcoding_audio_profile_new ();

  return ret;
}

gchar *
gst_transcoding_input_get_uri (GstTranscodingInput *self)
{
  return g_strdup (self->uri);
}

gchar *
gst_transcoding_output_get_uri (GstTranscodingOutput *self)
{
  return g_strdup (self->uri);
}

GstTranscodingContainerProfile *
gst_transcoding_output_get_profile (GstTranscodingOutput *self)
{
  return g_object_ref (self->profile);
}
