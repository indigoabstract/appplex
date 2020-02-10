#include "java-callbacks.h"
#include <jni.h>
#include <stdio.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

 
extern jobject  g_obj;
extern JavaVM*  g_p_java_vm;

void PreCall(JNIEnv** env, int* isAttached)
{
	(*isAttached) = 0;
	#ifdef __cplusplus
		int status = (g_p_java_vm)->GetEnv((void**)env, JNI_VERSION_1_4);
	#else
		int status = (*g_p_java_vm)->GetEnv(g_p_java_vm, (void**)env, JNI_VERSION_1_4);
	#endif
	if (status < 0)
	{
		#ifdef __cplusplus
			status = (g_p_java_vm)->AttachCurrentThread(env, NULL);
		#else
			status = (*g_p_java_vm)->AttachCurrentThread(g_p_java_vm, env, NULL);
		#endif
		if (status < 0)
		{
			return;
		}
		(*isAttached) = 1;
	}
}

void PostCall(int isAttached)
{
	if (isAttached)
	{
		#ifdef __cplusplus
			(g_p_java_vm)->DetachCurrentThread();
		#else
			(*g_p_java_vm)->DetachCurrentThread(g_p_java_vm);
		#endif
	}
}

jboolean CppBool_To_JniBool(int val)            { return (jboolean)((val) ? 1 : 0); };
jbyte    CppChar_To_JniByte(char val)           { return (jbyte   )  val; };
jchar    CppChar_To_JniChar(char val)           { return (jchar   )  val; };
jint     CppInt_To_JniInt(int val)              { return (jint    )  val; };
jlong    CppLong_To_JniLong(long long val)      { return (jlong   )  val; };
jfloat   CppFloat_To_JniFloat(float val)        { return (jfloat  )  val; };
jdouble  CppDouble_To_JniDouble(double val)     { return (jdouble )  val; };

jstring  CppString_to_JniString(const char* const str)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	#ifdef __cplusplus
		jstring jstr = (jstring)(env)->NewStringUTF(str);
	#else
		jstring jstr = (jstring)(*env)->NewStringUTF(env, str);
	#endif
	
	PostCall(isAttached);
	return jstr;
};

jobjectArray  CppArrString_to_JniArrString(char** arrStr, int size)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	#ifdef __cplusplus
		jobjectArray jarrstr = (jobjectArray)(env)->NewObjectArray( 
	#else
		jobjectArray jarrstr = (jobjectArray)(*env)->NewObjectArray(env, 
	#endif
								size,
	#ifdef __cplusplus	
								(env)->FindClass("java/lang/String"),
								(env)->NewStringUTF(""));
	#else
								(*env)->FindClass(env, "java/lang/String"),
								(*env)->NewStringUTF(env, ""));
	#endif

	PostCall(isAttached);
	return jarrstr;
};

void JavaCallMethod_void(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		(env)->CallVoidMethodV(g_obj, appMethodID, ap);
	#else
		(*env)->CallVoidMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
}

int JavaCallMethod_int(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetMethodID((*env),l_cls, name, prototype);
	#endif

	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jint jVal = (env)->CallIntMethodV(g_obj, appMethodID, ap);
	#else
		jint jVal = (*env)->CallIntMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
	return (jVal);
}

int JavaCallMethod_bool(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jboolean jVal = (env)->CallBooleanMethodV(g_obj, appMethodID, ap);
	#else
		jboolean jVal = (*env)->CallBooleanMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
	return (jVal) ? 1 : 0;
}

const char* const JavaCallMethod_string(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jstring jVal = (jstring)(env)->CallObjectMethodV(g_obj, appMethodID, ap);
	#else
		jstring jVal = (*env)->CallObjectMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	// process string...
	char*  result = NULL;
	if (NULL != jVal)
	{
		#ifdef __cplusplus
			const char* const str = (env)->GetStringUTFChars(jVal, 0);
		#else
			const char* const str = (*env)->GetStringUTFChars((*env),jVal, 0);
		#endif
		if (NULL != str)
		{
			result = (char*)malloc(strlen(str) + 1);
			sprintf(result, "%s", (char*)str);
		}
		#ifdef __cplusplus
			(env)->ReleaseStringUTFChars(jVal, str);
		#else
			(*env)->ReleaseStringUTFChars((*env),jVal, str);
		#endif
	}

	PostCall(isAttached);
	return result;
}

void JavaCallStaticMethodByObject_void(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		(env)->CallStaticVoidMethodV(l_cls, appMethodID, ap);
	#else
		(*env)->CallStaticVoidMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
}

//-------------------------------------------------------------------------
int JavaCallStaticMethodByObject_int(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jint jVal = (env)->CallStaticIntMethodV(l_cls, appMethodID, ap);
	#else
		jint jVal = (*env)->CallStaticIntMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
	return (jVal);
}

//-------------------------------------------------------------------------
int JavaCallStaticMethodByObject_bool(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jboolean jVal = (env)->CallStaticBooleanMethodV(l_cls, appMethodID, ap);
	#else
		jboolean jVal = (*env)->CallStaticBooleanMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
	return (jVal) ? 1 : 0;
}

//-------------------------------------------------------------------------
const char* const JavaCallStaticMethodByObject_string(const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass     l_cls       = (env)->GetObjectClass(g_obj);
	#else
		jclass     l_cls       = (*env)->GetObjectClass((*env),g_obj);
	#endif
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jstring jVal = (jstring)(env)->CallStaticObjectMethodV(l_cls, appMethodID, ap);
	#else
		jstring jVal = (*env)->CallStaticObjectMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	// process string...
	char*  result = NULL;
	if (NULL != jVal)
	{
		#ifdef __cplusplus
			const char* const str = (env)->GetStringUTFChars(jVal, 0);
		#else
			const char* const str = (*env)->GetStringUTFChars((*env),jVal, 0);
		#endif
		if (NULL != str)
		{
			result = (char*)malloc(strlen(str) + 1);
			sprintf(result, "%s", str);
		}
		#ifdef __cplusplus
			(env)->ReleaseStringUTFChars(jVal, str);
		#else
			(*env)->ReleaseStringUTFChars((*env),jVal, str);
		#endif
	}

	PostCall(isAttached);
	return result;
}

//*************************************************************************
void JavaCallStaticMethodByClass_void(const char* const javacls, const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass l_cls = (env)->FindClass(javacls);
	#else
		jclass l_cls = (*env)->FindClass((*env),javacls);
	#endif
	if (l_cls == NULL)
	{
//		_nwKrnOutDebug("!!! ERROR: JavaCallStaticMethod_void(...) - can not find java-class !!!");
		return;
	}
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		(env)->CallStaticVoidMethodV(l_cls, appMethodID, ap);
	#else
		(*env)->CallStaticVoidMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
}

//-------------------------------------------------------------------------
int JavaCallStaticMethodByClass_int(const char* const javacls, const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass l_cls = (env)->FindClass(javacls);
	#else
		jclass l_cls = (*env)->FindClass((*env),javacls);
	#endif
	if (l_cls == NULL)
	{
//		_nwKrnOutDebug("!!! ERROR: JavaCallStaticMethod_int(...) - can not find java-class !!!");
		return 0;
	}
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jint jVal = (env)->CallStaticIntMethodV(l_cls, appMethodID, ap);
	#else
		jint jVal = (*env)->CallStaticIntMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
	return (jVal);
}

//-------------------------------------------------------------------------
int JavaCallStaticMethodByClass_bool(const char* const javacls, const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass l_cls = (env)->FindClass(javacls);
	#else
		jclass l_cls = (*env)->FindClass((*env),javacls);
	#endif
	if (l_cls == NULL)
	{
//		_nwKrnOutDebug("!!! ERROR: JavaCallStaticMethod_bool(...) - can not find java-class !!!");
		return 0;
	}
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jboolean jVal = (env)->CallStaticBooleanMethodV(l_cls, appMethodID, ap);
	#else
		jboolean jVal = (*env)->CallStaticBooleanMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	PostCall(isAttached);
	return (jVal) ? 1 : 0;
}

//-------------------------------------------------------------------------
const char* const JavaCallStaticMethodByClass_string(const char* const javacls, const char* const name, const char* const prototype, ...)
{
	JNIEnv* env = NULL;
	int     isAttached = 0;
	PreCall(&env, &isAttached);

	// ask for trouble...
	#ifdef __cplusplus
		jclass l_cls = (env)->FindClass(javacls);
	#else
		jclass l_cls = (*env)->FindClass((*env),javacls);
	#endif
	if (l_cls == NULL)
	{
//		_nwKrnOutDebug("!!! ERROR: JavaCallStaticMethod_string(...) - can not find java-class !!!");
		return 0;
	}
	#ifdef __cplusplus
		jmethodID  appMethodID = (env)->GetStaticMethodID(l_cls, name, prototype);
	#else
		jmethodID  appMethodID = (*env)->GetStaticMethodID((*env),l_cls, name, prototype);
	#endif

	// make the call...
	va_list ap;
	va_start(ap, prototype);
	#ifdef __cplusplus
		jstring jVal = (jstring)(env)->CallStaticObjectMethodV(l_cls, appMethodID, ap);
	#else
		jstring jVal = (*env)->CallStaticObjectMethodV((*env),g_obj, appMethodID, ap);
	#endif
	va_end(ap);

	// process string...
	char*  result = NULL;
	if (NULL != jVal)
	{
		#ifdef __cplusplus
			const char* const str = (env)->GetStringUTFChars(jVal, 0);
		#else
			const char* const str = (*env)->GetStringUTFChars((*env),jVal, 0);
		#endif
		if (NULL != str)
		{
			result = (char*)malloc(strlen(str) + 1);
			sprintf(result, "%s", str);
		}
		#ifdef __cplusplus
			(env)->ReleaseStringUTFChars(jVal, str);
		#else
			(*env)->ReleaseStringUTFChars((*env),jVal, str);
		#endif
	}

	PostCall(isAttached);
	return result;
}

//-------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif
