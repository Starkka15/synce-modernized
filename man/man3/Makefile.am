real_MANS = synce_info_new.3 \
	synce_get_directory.3 \
	synce_get_connection_filename.3 \
	synce_strerror.3 \
	wstr_to_ascii.3 \
	time_fields_from_filetime.3 \
	date_from_tm.3 \
	synce_error.3 \
	rapi_connection_from_name.3

EXTRA_DIST = $(real_MANS)


INFO_LINKS = synce_info_destroy.3

GETDIR_LINKS = synce_get_subdirectory.3 \
		synce_get_script_directory.3

GETCONN_LINKS = synce_set_connection_filename.3 \
		synce_set_default_connection_filename.3

WSTR_LINKS = wstr_to_current.3 \
		wstr_to_utf8.3 \
		wstr_from_ascii.3 \
		wstr_from_utf8.3 \
		wstr_from_current.3 \
		wstr_free_string.3 \
		wstrlen.3 \
		wstrcpy.3 \
		wstr_append.3 \
		wstr_equal.3 \
		wstrdup.3

TIME_LINKS = time_fields_to_filetime.3

DATE_LINKS = date_to_tm.3

ERROR_LINKS = synce_warning.3 synce_info.3 synce_debug.3 synce_trace.3 synce_warning_unless.3 synce_log_set_level.3 synce_log_use_syslog.3

ALL_LINKS = $(INFO_LINKS) $(GETDIR_LINKS) $(GETCONN_LINKS) $(WSTR_LINKS) $(TIME_LINKS) $(DATE_LINKS) $(ERROR_LINKS)

CLEANFILES = $(ALL_LINKS)

man_MANS = $(real_MANS) $(ALL_LINKS)


$(INFO_LINKS): synce_info_new.3
	echo ".so man3/$^" > $@ 

$(GETDIR_LINKS): synce_get_directory.3
	echo ".so man3/$^" > $@ 

$(GETCONN_LINKS): synce_get_connection_filename.3
	echo ".so man3/$^" > $@ 

$(WSTR_LINKS): wstr_to_ascii.3
	echo ".so man3/$^" > $@ 

$(TIME_LINKS): time_fields_from_filetime.3
	echo ".so man3/$^" > $@ 

$(DATE_LINKS): date_from_tm.3
	echo ".so man3/$^" > $@ 

$(ERROR_LINKS): synce_error.3
	echo ".so man3/$^" > $@ 
