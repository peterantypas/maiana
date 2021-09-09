add_library(ais 
ais.cpp
ais_bitset.cpp
ais1_2_3.cpp
ais4_11.cpp
ais5.cpp
ais6.cpp
ais7_13.cpp
ais8.cpp
ais8_1_22.cpp
ais8_1_26.cpp
ais8_200.cpp
ais8_366.cpp
ais8_366_22.cpp
ais8_367.cpp
ais9.cpp
ais10.cpp
ais12.cpp
ais14.cpp
ais15.cpp
ais16.cpp
ais17.cpp
ais18.cpp
ais19.cpp
ais20.cpp
ais21.cpp
ais22.cpp
ais23.cpp
ais24.cpp
ais25.cpp
ais26.cpp
ais27.cpp
decode_body.cpp
vdm.cpp
)
target_include_directories(ais PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(ais PROPERTIES PUBLIC_HEADER "ais.h;vdm.h")

include(GNUInstallDirs)

install(TARGETS ais
  EXPORT LibaisConfig
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Not yet handled:
# ais_py.cpp
# ais_decode_normed.cpp 
