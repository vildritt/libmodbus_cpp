# lxqmt_getUserConfs
#     Usage:
#       add to qmakefeatures:
#
#         ...lxx/misc/lxqmtools 
#
#       add to pri/pro file of your subproject:
#
#         load(lxqmt, 1):defined(lxqmt_getUserConfs) {
#             LIB_USER_CONF_FILES = $$lxqmt_getUserConfs($${PWD}[, suffix])
#             for(LIB_USER_CONF_FILE, LIB_USER_CONF_FILES):include($$LIB_USER_CONF_FILE)
#         }
#

defineReplace(lxqmt_getUserConfs) {
    USER_CONF_FOLDERS = $$1
    USER_CONF_AUX_SUFFIX = $$2
    USER_CONF_SUFFIX =
    USER_CONF_TO_LOAD =
    for(USER_CONF_FOLDER, USER_CONF_FOLDERS) {
        USER_CONF_FOLDERS_LEVELS = 1 2 3 4
        for(USER_CONF_FOLDERS_LEVEL, USER_CONF_FOLDERS_LEVELS) {
            
            !equals($${USER_CONF_AUX_SUFFIX},): {
                LIB_LOCAL_USER_CONF = $${USER_CONF_FOLDER}/user_conf$${USER_CONF_SUFFIX}$${USER_CONF_AUX_SUFFIX}.pri
                exists($${LIB_LOCAL_USER_CONF}): USER_CONF_TO_LOAD = $${LIB_LOCAL_USER_CONF} $${USER_CONF_TO_LOAD}
            }
            
            LIB_LOCAL_USER_CONF = $${USER_CONF_FOLDER}/user_conf$${USER_CONF_SUFFIX}.pri
            exists($${LIB_LOCAL_USER_CONF}): USER_CONF_TO_LOAD = $${LIB_LOCAL_USER_CONF} $${USER_CONF_TO_LOAD}

            LIB_LOCAL_USER_CONF = $${USER_CONF_FOLDER}/user_conf$${USER_CONF_SUFFIX}.local.pri
            exists($${LIB_LOCAL_USER_CONF}): USER_CONF_TO_LOAD = $${LIB_LOCAL_USER_CONF} $${USER_CONF_TO_LOAD}

            USER_CONF_SUFFIX = .$$basename(USER_CONF_FOLDER)$${USER_CONF_AUX_SUFFIX}
            USER_CONF_FOLDER = $$dirname(USER_CONF_FOLDER)
        }
    }
    equals(LXT_QMAKE_DEBUG,Y):equals(USER_CONF_TO_LOAD,): warning([lxqmt][user_conf] no user conf found for feature [$$1]...)
    return($$USER_CONF_TO_LOAD)
}


defineTest(lxqmt_debugFeatureFound) {
    FEATURE_NAME = $$1
    equals(LXT_QMAKE_DEBUG,Y): warning([lxqmt][feat] ($$FEATURE_NAME) feature found!)
}


defineTest(lxqmt_deprecatedFile) {
    OLD_NAME = $$1
    SUGGESTION = $$2
    exists($$OLD_NAME): warning([lxqmt] using file [$$basename(OLD_NAME)] is depracated. $$SUGGESTION)
}
