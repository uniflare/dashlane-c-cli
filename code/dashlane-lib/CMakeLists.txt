cmake_minimum_required( VERSION 3.26.0 )

oct_define_sources(
    PLATFORM ALL

    "CMakeLists.txt"

    GROUP "include/dashlane"
        "include/dashlane/API.h"
        "include/dashlane/Dashlane.h"
        "include/dashlane/Errors.h"

    GROUP "src"
        "src/Dashlane.h"
        "src/Dashlane.cpp"
        "src/Database.h"
        "src/Database.cpp"
        "src/Encryption.h"
        "src/Encryption.cpp"
        "src/Keychain.h"
        "src/Keychain.cpp"
        "src/Serialization.h"
        "src/Serialization.cpp"
        "src/StdAfx.cpp"
        "src/StdAfx.h"

    GROUP "src/Api"
        "src/api/ApiRequest.h"
        "src/api/ApiRequest.cpp"

    GROUP "src/Api/Endpoints"
        "src/Api/Endpoints/CompleteDeviceRegistration.h"
        "src/Api/Endpoints/GetAuthenticationMethodsForDevice.h"
        "src/Api/Endpoints/GetLatestContent.h"
        "src/Api/Endpoints/PerformDashlaneAuthenticatorVerification.h"
        "src/Api/Endpoints/PerformDuoPushVerification.h"
        "src/Api/Endpoints/PerformEmailTokenVerification.h"
        "src/Api/Endpoints/PerformTotpVerification.h"
        "src/Api/Endpoints/RequestEmailTokenVerification.h"

    GROUP "src/Types"
        "src/Types/Auth.h"
        "src/Types/Crypto.h"
        "src/Types/Transactions.h"

    GROUP "src/Utility"
        "src/Utility/ConceptHelpers.h"
        "src/Utility/Cryptography.h"
        "src/Utility/Filesystem.h"
        "src/Utility/Strings.h"
        "src/Utility/Time.h"
        "src/Utility/Transaction.h"
        "src/Utility/Vector.h"
        "src/Utility/Zip.h"
)

oct_project(dashlane-lib TYPE STATIC FOLDER "Dashlane")

target_include_directories(${THIS_PROJECT}
    PUBLIC include
    PRIVATE src
    PRIVATE ${OCT_SDKS_DIR}/json/_src/include
    PRIVATE ${OCT_SDKS_DIR}/strutil/_src
)

target_link_libraries(${THIS_PROJECT}
    PRIVATE argon2
    PRIVATE base64pp
    PRIVATE curl
    PRIVATE keychain
    PRIVATE pugixml
    PRIVATE SQLiteCpp
    PRIVATE zlib
)

set_target_properties(${THIS_PROJECT} PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

target_precompile_headers(${THIS_PROJECT}
    PRIVATE
        src/StdAfx.h
)