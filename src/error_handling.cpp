/**
 * @brief error_handling
 * @author Nicholas Schloss <nicholas.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2020 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/
#include "hadoucan_util/error_handling.hpp"

#include <functional>
#include <cerrno>
#include <cstdlib>
#include <cstring>

const char * get_errno_msg(){
    int* local_err = nullptr;

    locale_t this_locale = uselocale(0);
    if(this_locale == static_cast<locale_t>(0)){
        return NULL;
    }
    const int err = errno;
    errno = 0;
    const char* msg = strerror_l(err, this_locale);

    if(local_err){
        *local_err = errno;
    }
    errno = err;

    return msg;
}
