# generated from catkin/cmake/template/pkgConfig.cmake.in

# append elements to a list and remove existing duplicates from the list
# copied from catkin/cmake/list_append_deduplicate.cmake to keep pkgConfig
# self contained
macro(_list_append_deduplicate listname)
  if(NOT "${ARGN}" STREQUAL "")
    if(${listname})
      list(REMOVE_ITEM ${listname} ${ARGN})
    endif()
    list(APPEND ${listname} ${ARGN})
  endif()
endmacro()

# append elements to a list if they are not already in the list
# copied from catkin/cmake/list_append_unique.cmake to keep pkgConfig
# self contained
macro(_list_append_unique listname)
  foreach(_item ${ARGN})
    list(FIND ${listname} ${_item} _index)
    if(_index EQUAL -1)
      list(APPEND ${listname} ${_item})
    endif()
  endforeach()
endmacro()

# pack a list of libraries with optional build configuration keywords
# copied from catkin/cmake/catkin_libraries.cmake to keep pkgConfig
# self contained
macro(_pack_libraries_with_build_configuration VAR)
  set(${VAR} "")
  set(_argn ${ARGN})
  list(LENGTH _argn _count)
  set(_index 0)
  while(${_index} LESS ${_count})
    list(GET _argn ${_index} lib)
    if("${lib}" MATCHES "^(debug|optimized|general)$")
      math(EXPR _index "${_index} + 1")
      if(${_index} EQUAL ${_count})
        message(FATAL_ERROR "_pack_libraries_with_build_configuration() the list of libraries '${ARGN}' ends with '${lib}' which is a build configuration keyword and must be followed by a library")
      endif()
      list(GET _argn ${_index} library)
      list(APPEND ${VAR} "${lib}${CATKIN_BUILD_CONFIGURATION_KEYWORD_SEPARATOR}${library}")
    else()
      list(APPEND ${VAR} "${lib}")
    endif()
    math(EXPR _index "${_index} + 1")
  endwhile()
endmacro()

# unpack a list of libraries with optional build configuration keyword prefixes
# copied from catkin/cmake/catkin_libraries.cmake to keep pkgConfig
# self contained
macro(_unpack_libraries_with_build_configuration VAR)
  set(${VAR} "")
  foreach(lib ${ARGN})
    string(REGEX REPLACE "^(debug|optimized|general)${CATKIN_BUILD_CONFIGURATION_KEYWORD_SEPARATOR}(.+)$" "\\1;\\2" lib "${lib}")
    list(APPEND ${VAR} "${lib}")
  endforeach()
endmacro()


if(wumpus_evaluation_CONFIG_INCLUDED)
  return()
endif()
set(wumpus_evaluation_CONFIG_INCLUDED TRUE)

# set variables for source/devel/install prefixes
if("TRUE" STREQUAL "TRUE")
  set(wumpus_evaluation_SOURCE_PREFIX /home/lisa/wumpusws/src/cnc-wumpus/wumpus_evaluation)
  set(wumpus_evaluation_DEVEL_PREFIX /home/lisa/wumpusws/src/cnc-wumpus/wumpus_evaluation/cmake-build-debug/devel)
  set(wumpus_evaluation_INSTALL_PREFIX "")
  set(wumpus_evaluation_PREFIX ${wumpus_evaluation_DEVEL_PREFIX})
else()
  set(wumpus_evaluation_SOURCE_PREFIX "")
  set(wumpus_evaluation_DEVEL_PREFIX "")
  set(wumpus_evaluation_INSTALL_PREFIX /usr/local)
  set(wumpus_evaluation_PREFIX ${wumpus_evaluation_INSTALL_PREFIX})
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "WARNING: package 'wumpus_evaluation' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  message("${_msg}")
endif()

# flag project as catkin-based to distinguish if a find_package()-ed project is a catkin project
set(wumpus_evaluation_FOUND_CATKIN_PROJECT TRUE)

if(NOT "/home/lisa/wumpusws/devel/.private/wumpus_simulator/include;/home/lisa/wumpusws/devel/.private/agent_id/include;/home/lisa/wumpusws/devel/.private/alica_msgs/include;/home/lisa/wumpusws/devel/.private/wumpus_msgs/include;/home/lisa/wumpusws/devel/.private/robot_control/include;/home/lisa/wumpusws/src/essentials/agent_id/include;/home/lisa/wumpusws/src/alica/alica_common_config/include;/home/lisa/wumpusws/src/alica/alica_solver_interface/include;/home/lisa/wumpusws/src/essentials/event_handling/include;/home/lisa/wumpusws/src/essentials/fsystem/include;/home/lisa/wumpusws/src/essentials/nonstd/include;/home/lisa/wumpusws/src/essentials/system_config/include;/home/lisa/wumpusws/src/essentials/system_util/include;/home/lisa/wumpusws/src/alica/alica_engine/include;/home/lisa/wumpusws/src/alica-supplementary/alica_ros_proxy/include;/home/lisa/wumpusws/src/aspsuite/asp_solver/include;/home/lisa/wumpusws/src/aspsuite/asp_solver_wrapper/include;/home/lisa/wumpusws/src/alica-supplementary/robot_control/include;/home/lisa/wumpusws/src/alica-supplementary/world_model/include;/home/lisa/wumpusws/src/wumpus-simulator/wumpus_simulator/include;/home/lisa/wumpusws/src/wumpus-simulator/wumpus_simulator/include/model;/home/lisa/wumpusws/src/cnc-wumpus/wumpus_worldmodel/include;/home/lisa/wumpusws/src/cnc-wumpus/wumpus_expressions/autogenerated/include;/opt/ros/kinetic/include;/opt/ros/kinetic/share/xmlrpcpp/cmake/../../../include/xmlrpcpp;/usr/include/x86_64-linux-gnu/qt5/;/usr/include/x86_64-linux-gnu/qt5/QtCore;/usr/lib/x86_64-linux-gnu/qt5//mkspecs/linux-g++-64;/usr/include/x86_64-linux-gnu/qt5/QtGui;/usr/include;/usr/include/x86_64-linux-gnu/qt5/QtNetwork;/usr/include/x86_64-linux-gnu/qt5/QtWidgets;/usr/include/x86_64-linux-gnu/qt5/QtWebKitWidgets;/usr/include/x86_64-linux-gnu/qt5/QtWebKit;/usr/local/include " STREQUAL " ")
  set(wumpus_evaluation_INCLUDE_DIRS "")
  set(_include_dirs "/home/lisa/wumpusws/devel/.private/wumpus_simulator/include;/home/lisa/wumpusws/devel/.private/agent_id/include;/home/lisa/wumpusws/devel/.private/alica_msgs/include;/home/lisa/wumpusws/devel/.private/wumpus_msgs/include;/home/lisa/wumpusws/devel/.private/robot_control/include;/home/lisa/wumpusws/src/essentials/agent_id/include;/home/lisa/wumpusws/src/alica/alica_common_config/include;/home/lisa/wumpusws/src/alica/alica_solver_interface/include;/home/lisa/wumpusws/src/essentials/event_handling/include;/home/lisa/wumpusws/src/essentials/fsystem/include;/home/lisa/wumpusws/src/essentials/nonstd/include;/home/lisa/wumpusws/src/essentials/system_config/include;/home/lisa/wumpusws/src/essentials/system_util/include;/home/lisa/wumpusws/src/alica/alica_engine/include;/home/lisa/wumpusws/src/alica-supplementary/alica_ros_proxy/include;/home/lisa/wumpusws/src/aspsuite/asp_solver/include;/home/lisa/wumpusws/src/aspsuite/asp_solver_wrapper/include;/home/lisa/wumpusws/src/alica-supplementary/robot_control/include;/home/lisa/wumpusws/src/alica-supplementary/world_model/include;/home/lisa/wumpusws/src/wumpus-simulator/wumpus_simulator/include;/home/lisa/wumpusws/src/wumpus-simulator/wumpus_simulator/include/model;/home/lisa/wumpusws/src/cnc-wumpus/wumpus_worldmodel/include;/home/lisa/wumpusws/src/cnc-wumpus/wumpus_expressions/autogenerated/include;/opt/ros/kinetic/include;/opt/ros/kinetic/share/xmlrpcpp/cmake/../../../include/xmlrpcpp;/usr/include/x86_64-linux-gnu/qt5/;/usr/include/x86_64-linux-gnu/qt5/QtCore;/usr/lib/x86_64-linux-gnu/qt5//mkspecs/linux-g++-64;/usr/include/x86_64-linux-gnu/qt5/QtGui;/usr/include;/usr/include/x86_64-linux-gnu/qt5/QtNetwork;/usr/include/x86_64-linux-gnu/qt5/QtWidgets;/usr/include/x86_64-linux-gnu/qt5/QtWebKitWidgets;/usr/include/x86_64-linux-gnu/qt5/QtWebKit;/usr/local/include")
  if(NOT " " STREQUAL " ")
    set(_report "Check the issue tracker '' and consider creating a ticket if the problem has not been reported yet.")
  elseif(NOT " " STREQUAL " ")
    set(_report "Check the website '' for information and consider reporting the problem.")
  else()
    set(_report "Report the problem to the maintainer 'lisa <lisa@todo.todo>' and request to fix the problem.")
  endif()
  foreach(idir ${_include_dirs})
    if(IS_ABSOLUTE ${idir} AND IS_DIRECTORY ${idir})
      set(include ${idir})
    elseif("${idir} " STREQUAL "include ")
      get_filename_component(include "${wumpus_evaluation_DIR}/../../../include" ABSOLUTE)
      if(NOT IS_DIRECTORY ${include})
        message(FATAL_ERROR "Project 'wumpus_evaluation' specifies '${idir}' as an include dir, which is not found.  It does not exist in '${include}'.  ${_report}")
      endif()
    else()
      message(FATAL_ERROR "Project 'wumpus_evaluation' specifies '${idir}' as an include dir, which is not found.  It does neither exist as an absolute directory nor in '/home/lisa/wumpusws/src/cnc-wumpus/wumpus_evaluation/${idir}'.  ${_report}")
    endif()
    _list_append_unique(wumpus_evaluation_INCLUDE_DIRS ${include})
  endforeach()
endif()

set(libraries "wumpus_evaluation")
foreach(library ${libraries})
  # keep build configuration keywords, target names and absolute libraries as-is
  if("${library}" MATCHES "^(debug|optimized|general)$")
    list(APPEND wumpus_evaluation_LIBRARIES ${library})
  elseif(TARGET ${library})
    list(APPEND wumpus_evaluation_LIBRARIES ${library})
  elseif(IS_ABSOLUTE ${library})
    list(APPEND wumpus_evaluation_LIBRARIES ${library})
  else()
    set(lib_path "")
    set(lib "${library}-NOTFOUND")
    # since the path where the library is found is returned we have to iterate over the paths manually
    foreach(path /home/lisa/wumpusws/src/cnc-wumpus/wumpus_evaluation/cmake-build-debug/devel/lib;/home/lisa/wumpusws/devel/lib;/opt/ros/kinetic/lib)
      find_library(lib ${library}
        PATHS ${path}
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
      if(lib)
        set(lib_path ${path})
        break()
      endif()
    endforeach()
    if(lib)
      _list_append_unique(wumpus_evaluation_LIBRARY_DIRS ${lib_path})
      list(APPEND wumpus_evaluation_LIBRARIES ${lib})
    else()
      # as a fall back for non-catkin libraries try to search globally
      find_library(lib ${library})
      if(NOT lib)
        message(FATAL_ERROR "Project '${PROJECT_NAME}' tried to find library '${library}'.  The library is neither a target nor built/installed properly.  Did you compile project 'wumpus_evaluation'?  Did you find_package() it before the subdirectory containing its code is included?")
      endif()
      list(APPEND wumpus_evaluation_LIBRARIES ${lib})
    endif()
  endif()
endforeach()

set(wumpus_evaluation_EXPORTED_TARGETS "")
# create dummy targets for exported code generation targets to make life of users easier
foreach(t ${wumpus_evaluation_EXPORTED_TARGETS})
  if(NOT TARGET ${t})
    add_custom_target(${t})
  endif()
endforeach()

set(depends "wumpus_simulator")
foreach(depend ${depends})
  string(REPLACE " " ";" depend_list ${depend})
  # the package name of the dependency must be kept in a unique variable so that it is not overwritten in recursive calls
  list(GET depend_list 0 wumpus_evaluation_dep)
  list(LENGTH depend_list count)
  if(${count} EQUAL 1)
    # simple dependencies must only be find_package()-ed once
    if(NOT ${wumpus_evaluation_dep}_FOUND)
      find_package(${wumpus_evaluation_dep} REQUIRED NO_MODULE)
    endif()
  else()
    # dependencies with components must be find_package()-ed again
    list(REMOVE_AT depend_list 0)
    find_package(${wumpus_evaluation_dep} REQUIRED NO_MODULE ${depend_list})
  endif()
  _list_append_unique(wumpus_evaluation_INCLUDE_DIRS ${${wumpus_evaluation_dep}_INCLUDE_DIRS})

  # merge build configuration keywords with library names to correctly deduplicate
  _pack_libraries_with_build_configuration(wumpus_evaluation_LIBRARIES ${wumpus_evaluation_LIBRARIES})
  _pack_libraries_with_build_configuration(_libraries ${${wumpus_evaluation_dep}_LIBRARIES})
  _list_append_deduplicate(wumpus_evaluation_LIBRARIES ${_libraries})
  # undo build configuration keyword merging after deduplication
  _unpack_libraries_with_build_configuration(wumpus_evaluation_LIBRARIES ${wumpus_evaluation_LIBRARIES})

  _list_append_unique(wumpus_evaluation_LIBRARY_DIRS ${${wumpus_evaluation_dep}_LIBRARY_DIRS})
  list(APPEND wumpus_evaluation_EXPORTED_TARGETS ${${wumpus_evaluation_dep}_EXPORTED_TARGETS})
endforeach()

set(pkg_cfg_extras "")
foreach(extra ${pkg_cfg_extras})
  if(NOT IS_ABSOLUTE ${extra})
    set(extra ${wumpus_evaluation_DIR}/${extra})
  endif()
  include(${extra})
endforeach()
