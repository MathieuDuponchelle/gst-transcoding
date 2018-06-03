exe = executable('test-job', 'job.c',
  dependencies: [gst_check_dep],
  include_directories: [inclib],
  link_with: libtranscoding,
)

test('job', exe)
