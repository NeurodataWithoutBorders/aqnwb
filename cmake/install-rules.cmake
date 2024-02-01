install(
    TARGETS aq-nwb_exe
    RUNTIME COMPONENT aq-nwb_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
