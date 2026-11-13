# These argument checks exit before opening a configuration or database.
foreach(_cli_server IN ITEMS authserver worldserver)
if(TARGET ${_cli_server})
  add_test(NAME ${_cli_server}_config_path_spaces
    COMMAND ${_cli_server} --version --config "${CMAKE_CURRENT_BINARY_DIR}/configuration path with spaces.conf")
  set_tests_properties(${_cli_server}_config_path_spaces PROPERTIES PASS_REGULAR_EXPRESSION "rev\\.")

  add_test(NAME ${_cli_server}_config_argument_required COMMAND ${_cli_server} --config)
  set_tests_properties(${_cli_server}_config_argument_required PROPERTIES
    WILL_FAIL TRUE
    FAIL_REGULAR_EXPRESSION "Error in config file")

  if(WIN32)
    get_filename_component(_cli_client_dll_dir "${MYSQL_LIBRARY}" DIRECTORY)
    get_filename_component(_cli_ssl_root "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
    set_tests_properties(${_cli_server}_config_path_spaces ${_cli_server}_config_argument_required PROPERTIES
      ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:${_cli_client_dll_dir};PATH=path_list_prepend:${_cli_ssl_root};PATH=path_list_prepend:${_cli_ssl_root}/bin")
  endif()
endif()
endforeach()
