add_library(sxm STATIC
    include/sxm/error/error.cpp
    include/sxm/parser/parse.cpp
    include/sxm/matcher/pattern_biop.cpp
    include/sxm/matcher/pattern_num.cpp
    include/sxm/matcher/pattern_ref.cpp
    include/sxm/matcher/pattern_scope_end.cpp

    include/sxm/ast/uop.cpp
    include/sxm/ast/biop.cpp
    include/sxm/ast/fnmap.cpp
    include/sxm/ast/numvalue.cpp

    include/sxm/eval/flat/eval.cpp
    include/sxm/eval/linked/eval.cpp
    include/sxm/eval/pure/ast.cpp
    include/sxm/eval/pure/eval.cpp
    include/sxm/eval/pure/build.cpp
    include/sxm/eval/compile/build.cpp
    include/sxm/eval/compile/unit.cpp
    include/sxm/eval/compile/declaration.cpp
)

add_library(sxm::sxm ALIAS sxm)

target_compile_features(sxm PUBLIC cxx_std_23)

target_include_directories(sxm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
