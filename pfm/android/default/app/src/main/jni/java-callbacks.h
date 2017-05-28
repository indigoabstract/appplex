#pragma once

#include <jni.h>


#ifdef __cplusplus
extern "C" {
#endif


jboolean CppBool_To_JniBool(int val);
jbyte    CppChar_To_JniByte(char val);
jchar    CppChar_To_JniChar(char val);
jint     CppInt_To_JniInt(int val);
jlong    CppLong_To_JniLong(long long val);
jfloat   CppFloat_To_JniFloat(float val);
jdouble  CppDouble_To_JniDouble(double val);

jstring       CppString_to_JniString(const char* const str);
jobjectArray  CppArrString_to_JniArrString(char** arrStr, int size);

void              JavaCallMethod_void  (const char* const name, const char* const prototype, ...);
int               JavaCallMethod_int   (const char* const name, const char* const prototype, ...);
int               JavaCallMethod_bool  (const char* const name, const char* const prototype, ...);
const char* const JavaCallMethod_string(const char* const name, const char* const prototype, ...);

void              JavaCallStaticMethodByObject_void  (const char* const name, const char* const prototype, ...);
int               JavaCallStaticMethodByObject_int   (const char* const name, const char* const prototype, ...);
int               JavaCallStaticMethodByObject_bool  (const char* const name, const char* const prototype, ...);
const char* const JavaCallStaticMethodByObject_string(const char* const name, const char* const prototype, ...);

void              JavaCallStaticMethodByClass_void  (const char* const javacls, const char* const name, const char* const prototype, ...);
int               JavaCallStaticMethodByClass_int   (const char* const javacls, const char* const name, const char* const prototype, ...);
int               JavaCallStaticMethodByClass_bool  (const char* const javacls, const char* const name, const char* const prototype, ...);
const char* const JavaCallStaticMethodByClass_string(const char* const javacls, const char* const name, const char* const prototype, ...);

#ifdef __cplusplus
}
#endif
