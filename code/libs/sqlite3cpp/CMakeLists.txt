set(THIS_SDK_DIR ${OCT_SDKS_DIR}/sqlite3cpp/_src)

# /// Bundled sqlite3
oct_define_sources(
	PLATFORM ALL
	
	GROUP "includes"
		"${THIS_SDK_DIR}/sqlite3/sqlite3.h"

	GROUP "sources"
		"${THIS_SDK_DIR}/sqlite3/sqlite3.c"
)
oct_project(sqlite3 LANGUAGE_C TYPE STATIC FOLDER "libs")

# /// SQLiteCpp
oct_define_sources(
	PLATFORM ALL

	"CMakeLists.txt"
	
	GROUP "includes"
		"${THIS_SDK_DIR}/include/SQLiteCpp/SQLiteCpp.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Assertion.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Backup.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Column.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Database.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Exception.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Savepoint.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Statement.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/Transaction.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/VariadicBind.h"
		"${THIS_SDK_DIR}/include/SQLiteCpp/ExecuteMany.h"

	GROUP "sources"
		"${THIS_SDK_DIR}/src/Backup.cpp"
		"${THIS_SDK_DIR}/src/Column.cpp"
		"${THIS_SDK_DIR}/src/Database.cpp"
		"${THIS_SDK_DIR}/src/Exception.cpp"
		"${THIS_SDK_DIR}/src/Savepoint.cpp"
		"${THIS_SDK_DIR}/src/Statement.cpp"
		"${THIS_SDK_DIR}/src/Transaction.cpp"
)

oct_project(SQLiteCpp TYPE STATIC FOLDER "libs")

target_include_directories(${THIS_PROJECT}
	PUBLIC ${THIS_SDK_DIR}/include
	PUBLIC ${THIS_SDK_DIR}/sqlite3)

target_link_libraries(SQLiteCpp PUBLIC sqlite3)

target_compile_options(${THIS_PROJECT}
	PUBLIC 
		"/wd4067" # unexpected tokens following preprocessor directive - expected a newline
)
