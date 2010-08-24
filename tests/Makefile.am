TESTS = \
	test-audio \
	test-property \
	test-properties \
	test-definition \
	test-event \
	test-request \
	test-sound-path \
	test-vibration-pattern \
	test-volume \
	test-context \
	test-volume-controller \
	test-audio-interface \
	test-dbus-if \
	test-backlight \
	test-backlight-dummy \
	test-led \
	test-led-dummy \
	test-tone-generator \
	test-state \
	test-settings


check_PROGRAMS = \
	test-audio \
	test-property \
	test-properties \
	test-definition \
	test-event \
	test-request \
	test-sound-path \
	test-vibration-pattern \
	test-volume \
	test-context \
	test-volume-controller \
	test-audio-interface \
	test-dbus-if \
	test-backlight \
	test-backlight-dummy \
	test-led \
	test-led-dummy \
	test-tone-generator \
	test-state \
	test-settings


INCLUDES = -I$(top_srcdir)/src

test_audio_SOURCES = test-audio.c $(top_srcdir)/src/audio.c $(top_srcdir)/src/audio-stream.h $(top_srcdir)/src/audio-interface.c $(top_srcdir)/src/audio-gstreamer.c $(top_srcdir)/src/log.c
test_audio_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@ @GST_CFLAGS@
test_audio_LDADD = @CHECK_LIBS@ @NGFD_LIBS@ @GST_LIBS@

test_property_SOURCES = test-property.c $(top_srcdir)/src/property.c
test_property_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_property_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_properties_SOURCES = test-properties.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/log.c $(top_srcdir)/src/property.c
test_properties_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_properties_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_definition_SOURCES = test-definition.c $(top_srcdir)/src/definition.c
test_definition_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_definition_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_event_SOURCES = test-event.c $(top_srcdir)/src/event.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/property.c $(top_srcdir)/src/log.c
test_event_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_event_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_request_SOURCES = test-request.c $(top_srcdir)/src/request.c $(top_srcdir)/src/context.c $(top_srcdir)/src/event.c  $(top_srcdir)/src/audio-stream.h  $(top_srcdir)/src/resources.h  $(top_srcdir)/src/sound-path.c  $(top_srcdir)/src/vibration-pattern.c $(top_srcdir)/src/vibrator-dummy.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/property.c  $(top_srcdir)/src/volume.c $(top_srcdir)/src/log.c
test_request_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_request_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_sound_path_SOURCES = test-sound-path.c $(top_srcdir)/src/sound-path.c
test_sound_path_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_sound_path_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_vibration_pattern_SOURCES = test-vibration-pattern.c $(top_srcdir)/src/vibration-pattern.c
test_vibration_pattern_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_vibration_pattern_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_volume_SOURCES = test-volume.c $(top_srcdir)/src/volume.c
test_volume_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_volume_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_context_SOURCES = test-context.c $(top_srcdir)/src/context.c $(top_srcdir)/src/log.c $(top_srcdir)/src/definition.c $(top_srcdir)/src/event.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/property.c $(top_srcdir)/src/request.c $(top_srcdir)/src/sound-path.c  $(top_srcdir)/src/vibration-pattern.c $(top_srcdir)/src/volume.c $(top_srcdir)/src/vibrator-dummy.c
test_context_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_context_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_volume_controller_SOURCES = test-volume-controller.c $(top_srcdir)/src/volume-controller.c $(top_srcdir)/src/volume.c $(top_srcdir)/src/log.c $(top_srcdir)/src/context.c $(top_srcdir)/src/definition.c $(top_srcdir)/src/event.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/property.c $(top_srcdir)/src/request.c $(top_srcdir)/src/sound-path.c  $(top_srcdir)/src/vibration-pattern.c $(top_srcdir)/src/vibrator-dummy.c
test_volume_controller_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_volume_controller_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_audio_interface_SOURCES = test-audio-interface.c $(top_srcdir)/src/audio-interface.c $(top_srcdir)/src/audio-stream.h $(top_srcdir)/src/volume.c $(top_srcdir)/src/audio-gstreamer.c $(top_srcdir)/src/log.c
test_audio_interface_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@ @GST_CFLAGS@
test_audio_interface_LDADD = @CHECK_LIBS@ @NGFD_LIBS@ @GST_LIBS@

test_dbus_if_SOURCES = test-dbus-if.c $(top_srcdir)/src/log.c $(top_srcdir)/src/property.c $(top_srcdir)/src/state.c $(top_srcdir)/src/context.c $(top_srcdir)/src/player.c $(top_srcdir)/src/definition.c $(top_srcdir)/src/event.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/request.c $(top_srcdir)/src/sound-path.c  $(top_srcdir)/src/vibration-pattern.c $(top_srcdir)/src/volume.c $(top_srcdir)/src/vibrator-dummy.c $(top_srcdir)/src/tone-generator.c $(top_srcdir)/src/led.c $(top_srcdir)/src/backlight.c $(top_srcdir)/src/audio.c $(top_srcdir)/src/audio-interface.c $(top_srcdir)/src/audio-stream.h $(top_srcdir)/src/audio-gstreamer.c $(top_srcdir)/src/tone-mapper.c
test_dbus_if_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@ @GST_CFLAGS@
test_dbus_if_LDADD = @CHECK_LIBS@ @NGFD_LIBS@ @GST_LIBS@

test_backlight_SOURCES = test-backlight.c $(top_srcdir)/src/log.c
test_backlight_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_backlight_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_backlight_dummy_SOURCES = test-backlight-dummy.c $(top_srcdir)/src/backlight-dummy.c $(top_srcdir)/src/log.c
test_backlight_dummy_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_backlight_dummy_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_led_SOURCES = test-led.c $(top_srcdir)/src/led.c $(top_srcdir)/src/log.c
test_led_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_led_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_led_dummy_SOURCES = test-led-dummy.c $(top_srcdir)/src/led-dummy.c $(top_srcdir)/src/log.c
test_led_dummy_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_led_dummy_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_tone_generator_SOURCES = test-tone-generator.c $(top_srcdir)/src/tone-generator.c $(top_srcdir)/src/log.c
test_tone_generator_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_tone_generator_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_state_SOURCES = test-state.c $(top_srcdir)/src/log.c $(top_srcdir)/src/player.c $(top_srcdir)/src/property.c $(top_srcdir)/src/timestamp.h $(top_srcdir)/src/resources.h $(top_srcdir)/src/request.c $(top_srcdir)/src/dbus-if.c $(top_srcdir)/src/sound-path.c $(top_srcdir)/src/led.c $(top_srcdir)/src/backlight.c $(top_srcdir)/src/tone-generator.c $(top_srcdir)/src/audio.c $(top_srcdir)/src/audio-interface.c $(top_srcdir)/src/audio-gstreamer.c $(top_srcdir)/src/tone-mapper.c $(top_srcdir)/src/vibrator-dummy.c $(top_srcdir)/src/vibration-pattern.c $(top_srcdir)/src/properties.c
test_state_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@ @GST_CFLAGS@
test_state_LDADD = @CHECK_LIBS@ @NGFD_LIBS@ @GST_LIBS@

test_settings_SOURCES = test-settings.c $(top_srcdir)/src/property.c $(top_srcdir)/src/properties.c $(top_srcdir)/src/event.c $(top_srcdir)/src/context.c $(top_srcdir)/src/log.c $(top_srcdir)/src/definition.c $(top_srcdir)/src/request.c $(top_srcdir)/src/sound-path.c  $(top_srcdir)/src/vibration-pattern.c $(top_srcdir)/src/volume.c $(top_srcdir)/src/vibrator-dummy.c
test_settings_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_settings_LDADD = @CHECK_LIBS@ @NGFD_LIBS@
