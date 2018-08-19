PHP_ARG_WITH(whais, Whether to enable WHAIS client support,
[ --with-whais   Enable Hello World support])

if test "$PHP_WHAIS" != "no"; then
  PHP_REQUIRE_CXX()
  AC_DEFINE(HAVE_WHAIS, 1, [Whether you have WHAIS client support])
  PHP_ADD_LIBRARY(stdc++, 1, WHAIS_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(wconnector, /home/ipopa/work/whisper/bin/linux_gcc_x86_64/client, WHAIS_SHARED_LIBADD)
fi

 PHP_SUBST(WHAIS_SHARED_LIBADD)
 PHP_NEW_EXTENSION(whais, whais.cc whais_params.cc, $ext_shared)
