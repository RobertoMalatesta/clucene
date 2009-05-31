/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_Array_
#define _lucene_util_Array_

#include <stdlib.h>
#include <string.h>

CL_NS_DEF(util)

template<typename T>
class CLUCENE_INLINE_EXPORT ArrayBase: LUCENE_BASE{
public:
	T* values;
	size_t length;
    
  /**
  * Delete's the values in the array. 
  * This won't do anything if deleteArray or takeArray is called first.
  * This is overridden in various implementations to provide the appropriate deletor function.
  */
  virtual void deleteValues(){
    if ( this->values == NULL )
      return;
	  for (size_t i=0;i<this->length;i++){
		  deleteValue(this->values[i]);
		}
  }
  /**
  * Delete's a single value. Used when resizing...
  */
  virtual void deleteValue(T) = 0;
	
  /**
  * Delete's the values in the array and then calls deleteArray(). 
  * This won't do anything if deleteArray or takeArray is called first.
  * This is overridden in various implementations to provide the appropriate deletor function.
  */
	void deleteAll(){ 
    this->deleteValues(); 
    this->deleteArray(); 
  }
	
	/**
	* Deletes the array holding the values. Do this if you want to take
	* ownership of the array's values, but not the array containing the values.
	*/
	void deleteArray(){
		free(this->values);
    this->values = NULL;
	}
	/**
	* Empties the array. Do this if you want to take ownership of the array
	* and the array's values.
	*/
	T* takeArray(){
	    T* ret = values;
	    values = NULL;
	    return ret;
	} 

	ArrayBase(){
		values = NULL;
		length = 0;
	}
	ArrayBase(T* values, size_t length){
		this->values = values;
		this->length = length;
	}
	ArrayBase(size_t length){
		this->values = (T*)malloc(sizeof(T)*length);
		this->length = length;
		if ( length > 0 ) memset(this->values,0,sizeof(T)*length);
	}
	virtual ~ArrayBase(){
	}

	const T& operator[](size_t _Pos) const
	{
		if (length <= _Pos){
			_CLTHROWA(CL_ERR_IllegalArgument,"vector subscript out of range");
		}
		return (*(values + _Pos));
	}
	T& operator[](size_t _Pos)
	{
		if (length <= _Pos){
			_CLTHROWA(CL_ERR_IllegalArgument,"vector subscript out of range");
		}
		return (*(values + _Pos));
	}
  
  /**
  * Resize the array
  * @param deleteValues if shrinking, delete the values that are lost.
  */
  void resize(size_t newSize, bool deleteValues=false){
    if ( length == newSize ) return;

    if ( deleteValues ){
      for ( size_t i=length;i<newSize;i++ ){
        deleteValue(values[i]);
      }
    }
    if ( newSize == 0 ){
      free(values);
      values = NULL;
    }else{
      if ( values == NULL ){
		    values = (T*)malloc(sizeof(T)*newSize);
      }else{
        values = (T*)realloc(values, sizeof(T) * newSize);
      }
    }
    if ( newSize > length )
      memset(values + length,0,sizeof(T) * (newSize-length));
    length = newSize;
  }
};

/**
* An array of objects. _CLDELETE is called on every contained object.
*/
template<typename T>
class CLUCENE_INLINE_EXPORT ObjectArray: public ArrayBase<T*>{
public:
  ObjectArray():ArrayBase<T*>(){}
	ObjectArray(T** values, size_t length):ArrayBase<T*>(values,length){}
	ObjectArray(size_t length):ArrayBase<T*>(length){}
	
  void deleteValues(){
      if ( this->values == NULL )
        return;
		  for (size_t i=0;i<this->length;i++){
			    _CLLDELETE(this->values[i]);
		  }
	    this->deleteArray();
	}
  void deleteValue(T* v){
     _CLLDELETE(v);
	}
	virtual ~ObjectArray(){
	    deleteValues();
	}

	/* Initializes all cells in the array with a NULL value */
	void initArray(){
		for (size_t i=0;i<this->length;i++){
			this->values[i]=NULL;
		}
	}
};

/**
* Legacy code... don't use, remove all instances of this!
*/
template<typename T>
class CLUCENE_INLINE_EXPORT Array: public ArrayBase<T>{
public:
  _CL_DEPRECATED(ObjectArray or ValueArray) Array():ArrayBase<T>(){}
	_CL_DEPRECATED(ObjectArray or ValueArray) Array(T* values, size_t length):ArrayBase<T>(values,length){}
	_CL_DEPRECATED(ObjectArray or ValueArray) Array(size_t length):ArrayBase<T>(length){}
  void deleteValues(){
    if ( this->values == NULL )
        return;
    this->deleteArray();
	}
  void deleteValue(T v){} //nothing to do...
	virtual ~Array(){
	}
};

/**
* An array where the values do not need to be deleted
*/
template<typename T>
class CLUCENE_INLINE_EXPORT ValueArray: public ArrayBase<T>{
public:
  ValueArray():ArrayBase<T>(){}
	ValueArray(T* values, size_t length):ArrayBase<T>(values,length){}
	ValueArray(size_t length):ArrayBase<T>(length){}

  void deleteValues(){
    if ( this->values == NULL )
        return;
    this->deleteArray();
	}
  void deleteValue(T v){} //nothing to do...
  virtual ~ValueArray(){/*deleteValues();*/}
};

/** A value array for const values (never deleted) */
template<typename T>
class CLUCENE_INLINE_EXPORT ConstValueArray: public ArrayBase<T>{
public:
  ConstValueArray():ArrayBase<T>(){}
	ConstValueArray(T* values, size_t length):ArrayBase<T>(values,length){}
	ConstValueArray(size_t length):ArrayBase<T>(length){}

  void deleteValues(){
	}
  void deleteValue(T v){} //nothing to do...
	virtual ~ConstValueArray(){
	}
};


/**
* An array of TCHAR strings
*/
class CLUCENE_INLINE_EXPORT TCharArray: public ArrayBase<TCHAR*>{
public:
    TCharArray():ArrayBase<TCHAR*>(){}
	TCharArray(TCHAR** values, size_t length):ArrayBase<TCHAR*>(values,length){}
	TCharArray(size_t length):ArrayBase<TCHAR*>(length){}

  void deleteValues(){
    if ( this->values == NULL )
        return;
	  for (size_t i=0;i<this->length;i++)
		  _CLDELETE_CARRAY(this->values[i]);
    this->deleteArray();
	}
  void deleteValue(TCHAR* v){
		  _CLDELETE_LCARRAY(v);
  }
	virtual ~TCharArray(){
	    deleteValues();
	}
};

/**
* An array of char strings
*/
class CLUCENE_INLINE_EXPORT CharArray: public ArrayBase<char*>{
public:
    CharArray():ArrayBase<char*>(){}
	CharArray(char** values, size_t length):ArrayBase<char*>(values,length){}
	CharArray(size_t length):ArrayBase<char*>(length){}

    void deleteValues(){
      if ( this->values == NULL )
          return;
		  for (size_t i=0;i<this->length;i++)
			  _CLDELETE_CaARRAY(this->values[i]);
	    this->deleteArray();
	}
	virtual ~CharArray(){
	    deleteValues();
	}
};

/**
* An array of const TCHAR strings
*/
class CLUCENE_INLINE_EXPORT TCharConstArray: public ArrayBase<const TCHAR*>{
public:
    TCharConstArray():ArrayBase<const TCHAR*>(){}
	TCharConstArray(const TCHAR** values, size_t length):ArrayBase<const TCHAR*>(values,length){}
	TCharConstArray(size_t length):ArrayBase<const TCHAR*>(length){}

    void deleteValues(){
        if ( this->values == NULL )
            return;
	    this->deleteArray();
	}
	virtual ~TCharConstArray(){
	    deleteValues();
	}
};

/**
* An array of const char strings
*/
class CLUCENE_INLINE_EXPORT CharConstArray: public ArrayBase<const char*>{
public:
    CharConstArray():ArrayBase<const char*>(){}
	CharConstArray(const char** values, size_t length):ArrayBase<const char*>(values,length){}
	CharConstArray(size_t length):ArrayBase<const char*>(length){}

    void deleteValues(){
        if ( this->values == NULL )
            return;
	    this->deleteArray();
	}
	virtual ~CharConstArray(){
	    deleteValues();
	}
};


CL_NS_END
#endif
