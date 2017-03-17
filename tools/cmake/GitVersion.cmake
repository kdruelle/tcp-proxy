


FIND_PACKAGE(Git)
IF(GIT_FOUND)
    MESSAGE("-- git found: ${GIT_EXECUTABLE}")

    IF(EXISTS "${PROJECT_SOURCE_DIR}/.git")
        # Get the latest abbreviated commit hash of the working branch
        execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        # Get the commit number
        execute_process(
            COMMAND git rev-list HEAD --count
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_NUMBER
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        # Get the current branch name
        execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_CUR_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        # Get the current tag name
        execute_process(
            COMMAND git describe --tags --exact-match
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_CUR_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )


        execute_process(
            COMMAND git describe --tags
            COMMAND cut -d - -f 1
            COMMAND sed "s/v//g"
            COMMAND cut -d . -f 1
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE APPLICATION_VERSION_MAJOR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        execute_process(
            COMMAND git describe --tags
            COMMAND cut -d - -f 1
            COMMAND sed "s/v//g"
            COMMAND cut -d . -f 2
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE APPLICATION_VERSION_MINOR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        execute_process(
            COMMAND git describe --tags
            COMMAND cut -d - -f 1
            COMMAND sed "s/v//g"
            COMMAND cut -d . -f 3
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE APPLICATION_VERSION_PATCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        execute_process(
            COMMAND git show -s --format=%ci ${GIT_COMMIT_HASH}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_VERSION_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        execute_process(
            COMMAND date -u --date "${GIT_VERSION_DATE}" "+%Y-%m-%d %H:%M:%S.%N"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_VERSION_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        SET (APPLICATION_VERSION_STRING "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}")
        SET (PROJECT_VERSION_STRING "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}")

    ELSE()
        MESSAGE("-- Not a git directory")
    ENDIF()


endif()


