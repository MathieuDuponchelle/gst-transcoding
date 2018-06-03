#include <gst/check/gstcheck.h>
#include <gst/transcoding/job.h>

GST_START_TEST (test_create_and_free)
{
  GstTranscodingJob *job = gst_transcoding_job_new ();

  g_object_unref (job);
}

GST_END_TEST;

/*
  {
    "outputs" : [
      {
        "uri" : "file:///foo/baz.mkv",
        "autolink" : false,
        "container-profile" : {
          "format" : "video/x-matroska",
          "meta-audio-profile" : {
            "format" : "application/unknown"
          },
          "meta-video-profile" : {
            "format" : "application/unknown"
          }
        }
      }
    ],
    "inputs" : [
      {
        "uri" : "file:///foo/bar",
        "autolink" : false,
        "streams" : [
          {
            "stream-id" : "stream-id",
            "media-type" : "video",
            "profiles" : [
              {
                "format" : "application/unknown",
                "output" : "file:///foo/baz.mkv"
              },
              {
                "format" : "application/unknown",
                "output" : "file:///foo/baz.mkv"
              }
            ]
          }
        ]
      }
    ]
  }
*/
GST_START_TEST (test_manual_mapping)
{
  GstTranscodingJob *job = gst_transcoding_job_new ();
  GstTranscodingVideoProfile *vprof, *vprof2;
  GstTranscodingInput *input, *input2;
  gchar *uri;
  GstTranscodingOutput *output, *output2;
  GstTranscodingFormat format;
  GstTranscodingAudioProfile *aprof;

  vprof = gst_transcoding_job_map_video_stream (job, "file:///foo/bar", "stream-id", "file:///foo/baz.mkv");
  fail_unless (vprof != NULL);

  /* Check the implicitly created input */
  input = gst_transcoding_stream_profile_get_input (GST_TRANSCODING_STREAM_PROFILE (vprof));
  fail_unless (input != NULL);
  uri = gst_transcoding_input_get_uri (input);
  fail_unless_equals_string (uri, "file:///foo/bar");
  g_object_unref (input);
  g_free (uri);

  /* Check the implicitly created output */
  output = gst_transcoding_stream_profile_get_output (GST_TRANSCODING_STREAM_PROFILE (vprof));
  fail_unless (output != NULL);
  uri = gst_transcoding_output_get_uri (output);
  fail_unless_equals_string (uri, "file:///foo/baz.mkv");
  g_object_unref (input);
  g_free (uri);

  /* Make sure creating this input is no longer possible */
  fail_unless (gst_transcoding_job_add_input (job, "file:///foo/bar") == NULL);

  /* Make sure creating this output is no longer possible */
  fail_unless (gst_transcoding_job_add_output (job, "file:///foo/baz.mkv", NULL) == NULL);

  /* The container profile has been implicitly created and we haven't set a
   * format on the video profile, as such its format should be None, which
   * means passthrough */
  format = gst_transcoding_stream_profile_get_format (GST_TRANSCODING_STREAM_PROFILE (vprof));
  fail_unless (format == GST_TRANSCODING_FORMAT_NONE);

  g_object_unref (vprof);

  /* Creating a new video profile with the same input / output is allowed */
  vprof2 = gst_transcoding_job_map_video_stream (job, "file:///foo/bar", "stream-id", "file:///foo/baz.mkv");
  fail_unless (vprof2 != NULL);

  /* Its input should be identical to the input of the first profile */
  input2 = gst_transcoding_stream_profile_get_input (GST_TRANSCODING_STREAM_PROFILE (vprof2));
  fail_unless (input2 == input);
  g_object_unref (input2);

  /* Its output should be identical to the output of the first profile */
  output2 = gst_transcoding_stream_profile_get_output (GST_TRANSCODING_STREAM_PROFILE (vprof2));
  fail_unless (output2 == output);
  g_object_unref (output2);

  g_object_unref (vprof2);

  /* It should however now be impossible to create an audio profile with the
   * same input / stream-id */
  aprof = gst_transcoding_job_map_audio_stream (job, "file:///foo/bar", "stream-id", "file:///foo/baz.mkv");
  fail_unless (aprof == NULL);

  g_object_unref (job);
}

GST_END_TEST;

/*
  {
    "outputs" : [
      {
        "uri" : "file:///foo/baz.mkv",
        "autolink" : true,
        "container-profile" : {
          "format" : "application/unknown",
          "meta-audio-profile" : {
            "format" : "application/unknown"
          },
          "meta-video-profile" : {
            "format" : "application/unknown"
          }
        }
      },
      {
        "uri" : "file://foo/baz.mp4",
        "autolink" : true,
        "container-profile" : {
          "format" : "video/x-matroska",
          "meta-audio-profile" : {
            "format" : "audio/x-aac"
          },
          "meta-video-profile" : {
            "format" : "video/x-h264"
          }
        }
      }
    ],
    "inputs" : [
      {
        "uri" : "file:///foo/bar",
        "autolink" : true,
        "streams" : [
        ]
      }
    ]
  }
*/
GST_START_TEST (test_automatic_mapping)
{
  GstTranscodingJob *job = gst_transcoding_job_new ();
  GstTranscodingInput *input;
  GstTranscodingOutput *output, *output2;
  GstTranscodingContainerProfile *auto_cprof, *cprof;
  GstTranscodingVideoProfile *auto_meta_vprof, *meta_vprof;
  GstTranscodingAudioProfile *meta_aprof;

  input = gst_transcoding_job_add_input (job, "file:///foo/bar");
  fail_unless (input != NULL);
  g_object_unref (input);

  output = gst_transcoding_job_add_output (job, "file:///foo/baz.mkv", NULL);
  fail_unless (output != NULL);

  /* A container profile was automatically created based on the file extension */
  auto_cprof = gst_transcoding_output_get_profile (output);
  g_object_unref (output);

  fail_unless (auto_cprof != NULL);
  fail_unless (gst_transcoding_container_profile_get_format (auto_cprof) == GST_TRANSCODING_FORMAT_MATROSKA);

  /* Meta stream profiles should have been automatically created */
  auto_meta_vprof = gst_transcoding_container_profile_get_meta_video_profile (auto_cprof);
  fail_unless (auto_meta_vprof != NULL);
  /* Automatically created stream profiles should be passthrough */
  fail_unless (gst_transcoding_stream_profile_get_format (GST_TRANSCODING_STREAM_PROFILE (auto_meta_vprof)) == GST_TRANSCODING_FORMAT_NONE);
  /* Meta profiles have no input */
  fail_unless (gst_transcoding_stream_profile_get_input (GST_TRANSCODING_STREAM_PROFILE (auto_meta_vprof)) == NULL);
  /* Meta profiles have no output */
  fail_unless (gst_transcoding_stream_profile_get_output (GST_TRANSCODING_STREAM_PROFILE (auto_meta_vprof)) == NULL);

  g_object_unref (auto_meta_vprof);

  /* Setting a different container format is OK */
  gst_transcoding_container_profile_set_format (auto_cprof, GST_TRANSCODING_FORMAT_NONE);

  fail_unless (gst_transcoding_container_profile_get_format (auto_cprof) == GST_TRANSCODING_FORMAT_NONE);

  g_object_unref (auto_cprof);

  /* It is impossible to create the same input twice */
  fail_unless (gst_transcoding_job_add_input (job, "file:///foo/bar") == NULL);

  /* It is impossible to create the same output twice */
  fail_unless (gst_transcoding_job_add_output (job, "file:///foo/baz.mkv", NULL) == NULL);

  meta_vprof = gst_transcoding_video_profile_new ();
  gst_transcoding_stream_profile_set_format (GST_TRANSCODING_STREAM_PROFILE (meta_vprof), GST_TRANSCODING_FORMAT_H264);
  meta_aprof = gst_transcoding_audio_profile_new ();
  gst_transcoding_stream_profile_set_format (GST_TRANSCODING_STREAM_PROFILE (meta_aprof), GST_TRANSCODING_FORMAT_AAC);

  /* Takes ownership of the meta profiles */
  cprof = gst_transcoding_container_profile_new (meta_aprof, meta_vprof);

  gst_transcoding_container_profile_set_format (cprof, GST_TRANSCODING_FORMAT_MATROSKA);

  /* Takes ownership of the container profile */
  output2 = gst_transcoding_job_add_output (job, "file://foo/baz.mp4", cprof);
  fail_unless (output2 != NULL);

  cprof = gst_transcoding_output_get_profile (output2);
  g_object_unref (output2);

  fail_unless (cprof != NULL);

  /* The file extension should not have been taken into account */
  fail_unless (gst_transcoding_container_profile_get_format (cprof) == GST_TRANSCODING_FORMAT_MATROSKA);

  /* Check the format of the meta audio profile */
  meta_aprof = gst_transcoding_container_profile_get_meta_audio_profile (cprof);
  fail_unless (gst_transcoding_stream_profile_get_format (GST_TRANSCODING_STREAM_PROFILE (meta_aprof)) == GST_TRANSCODING_FORMAT_AAC);
  g_object_unref (meta_aprof);

  /* Check the format of the meta video profile */
  meta_vprof = gst_transcoding_container_profile_get_meta_video_profile (cprof);
  fail_unless (gst_transcoding_stream_profile_get_format (GST_TRANSCODING_STREAM_PROFILE (meta_vprof)) == GST_TRANSCODING_FORMAT_H264);
  g_object_unref (meta_vprof);

  g_object_unref (cprof);

  g_object_unref (job);
}

GST_END_TEST;

/*
  {
    "outputs" : [
      {
        "uri" : "file://foo/baz.mkv",
        "autolink" : true,
        "container-profile" : {
          "format" : "video/x-matroska",
          "meta-audio-profile" : {
            "format" : "audio/x-aac"
          },
          "meta-video-profile" : {
            "format" : "video/x-h264"
          }
        }
      }
    ],
    "inputs" : [
      {
        "uri" : "file:///foo/bar",
        "autolink" : true,
        "streams" : [
          {
            "stream-id" : "audio-stream-id",
            "media-type" : "audio",
            "profiles" : [
              {
                "format" : "audio/x-aac",
                "output" : "file://foo/baz.mkv"
              }
            ]
          },
          {
            "stream-id" : "video-stream-id",
            "media-type" : "video",
            "profiles" : [
              {
                "format" : "video/x-h264",
                "output" : "file://foo/baz.mkv"
              }
            ]
          }
        ]
      }
    ]
  }
*/
GST_START_TEST (test_hybrid_mapping)
{
  GstTranscodingJob *job = gst_transcoding_job_new ();
  GstTranscodingInput *input;
  GstTranscodingOutput *output;
  GstTranscodingContainerProfile *cprof;
  GstTranscodingVideoProfile *meta_vprof;
  GstTranscodingAudioProfile *meta_aprof;
  GstTranscodingVideoProfile *vprof;
  GstTranscodingAudioProfile *aprof;

  input = gst_transcoding_job_add_input (job, "file:///foo/bar");
  g_object_unref (input);

  meta_vprof = gst_transcoding_video_profile_new ();
  gst_transcoding_stream_profile_set_format (GST_TRANSCODING_STREAM_PROFILE (meta_vprof), GST_TRANSCODING_FORMAT_H264);
  meta_aprof = gst_transcoding_audio_profile_new ();
  gst_transcoding_stream_profile_set_format (GST_TRANSCODING_STREAM_PROFILE (meta_aprof), GST_TRANSCODING_FORMAT_AAC);
  cprof = gst_transcoding_container_profile_new (meta_aprof, meta_vprof);
  gst_transcoding_container_profile_set_format (cprof, GST_TRANSCODING_FORMAT_MATROSKA);
  output = gst_transcoding_job_add_output (job, "file://foo/baz.mkv", cprof);
  g_object_unref (output);

  vprof = gst_transcoding_job_map_video_stream (job, "file:///foo/bar", "video-stream-id", "file://foo/baz.mkv");

  /* We check that the video profile was created from the meta video profile */
  fail_unless (gst_transcoding_stream_profile_get_format (GST_TRANSCODING_STREAM_PROFILE (vprof)) == GST_TRANSCODING_FORMAT_H264);
  g_object_unref (vprof);

  aprof = gst_transcoding_job_map_audio_stream (job, "file:///foo/bar", "audio-stream-id", "file://foo/baz.mkv");

  /* We check that the audio profile was created from the meta audio profile */
  fail_unless (gst_transcoding_stream_profile_get_format (GST_TRANSCODING_STREAM_PROFILE (aprof)) == GST_TRANSCODING_FORMAT_AAC);
  g_object_unref (aprof);

  g_object_unref (job);
}

GST_END_TEST;

static Suite *
gst_transcoding_job_suite (void)
{
  Suite *s = suite_create ("GstTranscodingJob");
  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_create_and_free);
  tcase_add_test (tc_chain, test_manual_mapping);
  tcase_add_test (tc_chain, test_automatic_mapping);
  tcase_add_test (tc_chain, test_hybrid_mapping);

  return s;
}

GST_CHECK_MAIN (gst_transcoding_job);
