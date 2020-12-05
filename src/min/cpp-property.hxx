#pragma once

/**
*	FILE: Cpp_Property.h
*	ARTHOR: Jeff Tanner, jeff_tanner@earthlink.net
*	CLASS NAME: Property
*	VERSION: 1.0
*	DESCRIPTION:
*		C++ implementation of C# Property functionality.
*/

#include "pfm-def.h"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
#include <string>
mws_pop_disable_all_warnings


/*!
* \brief
* Defines expected interface for Accessor's
* set and get member functions.
*/
template <class Host, typename ValueType> class IAccessor
{
public:
   virtual void set(Host*, const ValueType&) = 0;
   virtual const ValueType& get(Host*) const = 0;
};

/*!
* \brief
* Defines an abstract base for Accessor
* providing value type used.
*/
template <class host, typename ValueType> class accessor_base : public IAccessor<host, ValueType>
{
public:
   typedef ValueType value_type;

   accessor_base()
   {
      val_changed = false;
   }

   virtual void set(host* /*i_host*/, const value_type& i_value) override
   {
      if (val != i_value)
      {
         val_changed = true;
         val = i_value;
      }
   }

   virtual const value_type& get(host* /*i_host*/) const override
   {
      return val;
   }

   const value_type& read(host* /*i_host*/)
   {
      val_changed = false;
      return val;
   }

   bool value_changed()const
   {
      return val_changed;
   }

protected:
   ValueType val;
   bool val_changed;
};

/*!
* \brief
* Defines expected member functions for Properties
* for the purpose of access to the Accessor's
* set and get member functions.
*/
template<typename ValueType> class IProperty
{
   /*!
   * \note Overload the '=' sign to set the value using the set member.
   */
   virtual const ValueType& operator =(const ValueType& value) = 0;

   /*!
   * \note To make possible to cast the property class to the internal type.
   */
   virtual operator const ValueType& () const = 0;
   virtual const ValueType& operator ()() const = 0;
};

/*!
* \brief
* Property's Base class.
*
* Defines the access to the Accessor's set and
* get member functions.
*
* Plus maintains a reference to its assigned
* Host reference to its Accessor.
*/
template<class Host, class Accessor, typename ValueType> class PropertyBase : public IProperty<ValueType>
{
public:
   PropertyBase(Host* i_host) : host_ref(i_host) {}

   virtual const ValueType& operator =(const ValueType& i_value) override
   {
      this->accessor_inst.set(this->host_ref, i_value);
      return i_value;
   }

   virtual operator const ValueType& () const override
   {
      return this->accessor_inst.get(this->host_ref);
   }

   virtual const ValueType& operator ()() const override
   {
      return this->accessor_inst.get(this->host_ref);
   }

   const ValueType& read()
   {
      return this->accessor_inst.read(this->host_ref);
   }

   bool value_changed()const
   {
      return this->accessor_inst.value_changed();
   }

protected:
   // ref only
   Host* host_ref;
   Accessor accessor_inst;
};


/*!
* \brief
* Property with public access to both set
* and get functions of its Accessor.
*/
template<class Host, class Accessor, typename ValueType> class Property : public PropertyBase<Host, Accessor, ValueType>
{
public:
   Property(Host* i_host = nullptr) : PropertyBase<Host, Accessor, ValueType>(i_host) {}

   using PropertyBase<Host, Accessor, ValueType>::operator =;
   using PropertyBase<Host, Accessor, ValueType>::operator const ValueType&;
   using PropertyBase<Host, Accessor, ValueType>::operator ();
};

/*!
* \brief
* Property with public access to set function of its Accessor
* and private access to get function of its Accessor.
*/
template<class Host, class Accessor, typename ValueType> class Property_Set_Public : public PropertyBase<Host, Accessor, ValueType>
{
   friend Host;

public:
   Property_Set_Public(Host* i_host = nullptr) : PropertyBase<Host, Accessor, ValueType>(i_host) {}

public:
   using PropertyBase<Host, Accessor, ValueType>::operator =;
private:
   using PropertyBase<Host, Accessor, ValueType>::operator const ValueType&;
};

/*!
* \brief
* Property with private access to set function of Accessor
* and public access to get function of Accessor.
*/
template<class Host, class Accessor, typename ValueType> class Property_Get_Public : public PropertyBase<Host, Accessor, ValueType>
{
   friend Host;

public:
   Property_Get_Public(Host* i_host = nullptr) : PropertyBase<Host, Accessor, ValueType>(i_host) {}

private:
   using PropertyBase<Host, Accessor, ValueType>::operator =;
public:
   using PropertyBase<Host, Accessor, ValueType>::operator const ValueType&;
};


template <class host> class bool_accessor : public accessor_base<host, bool>
{
public:
};


template <class host, typename ValueType> class number_accessor : public accessor_base<host, ValueType>
{
public:
   virtual void add(host* i_host, const ValueType& i_value)
   {
      this->set(i_host, this->val + i_value);
   }

   virtual void mul(host* i_host, const ValueType& i_value)
   {
      this->set(i_host, this->val * i_value);
   }

   virtual void div(host* i_host, const ValueType& i_value)
   {
      this->set(i_host, this->val / i_value);
   }
};

template <class host> class vec3_accessor : public number_accessor<host, glm::vec3>
{
public:
   virtual void set(host* i_host, const glm::vec3& i_value)
   {
      number_accessor<host, glm::vec3>::set(i_host, i_value);
   }
};

template <class host> class mat4_accessor : public number_accessor<host, glm::mat4>
{
public:
   virtual void set(host* i_host, const glm::mat4& i_value)
   {
      number_accessor<host, glm::mat4>::set(i_host, i_value);
   }
};

template<class Host, class Accessor, typename ValueType> class number_property : public Property<Host, Accessor, ValueType>
{
public:
   number_property(Host* i_host = nullptr) : Property<Host, Accessor, ValueType>(i_host) {}

   virtual operator const ValueType& () const
   {
      return this->accessor_inst.get(this->host_ref);
   }

   virtual const ValueType& operator =(const ValueType& i_value)
   {
      this->accessor_inst.set(this->host_ref, i_value);
      return i_value;
   }

   virtual const ValueType& operator ()() const
   {
      return this->accessor_inst.get(this->host_ref);
   }

   virtual const ValueType& operator +=(const ValueType& i_value)
   {
      this->accessor_inst.add(this->host_ref, i_value);
      return this->accessor_inst.get(this->host_ref);
   }

   virtual const ValueType& operator -=(const ValueType& i_value)
   {
      this->accessor_inst.add(this->host_ref, -i_value);
      return this->accessor_inst.get(this->host_ref);
   }

   virtual const ValueType& operator *=(const ValueType& i_value)
   {
      this->accessor_inst.mul(this->host_ref, i_value);
      return this->accessor_inst.get(this->host_ref);
   }

   virtual const ValueType& operator /=(const ValueType& i_value)
   {
      this->accessor_inst.div(this->host_ref, i_value);
      return this->accessor_inst.get(this->host_ref);
   }

   virtual ValueType operator+(const ValueType& i_value) { return this->accessor_inst.get(this->host_ref) + i_value; }
   virtual ValueType operator-(const ValueType& i_value) { return this->accessor_inst.get(this->host_ref) - i_value; }
   virtual ValueType operator*(const ValueType& i_value) { return this->accessor_inst.get(this->host_ref) * i_value; }
   virtual ValueType operator/(const ValueType& i_value) { return this->accessor_inst.get(this->host_ref) / i_value; }
   friend ValueType operator+(const ValueType& i_v0, const number_property& i_v1) { return i_v0 + (ValueType)i_v1; }
   friend ValueType operator-(const ValueType& i_v0, const number_property& i_v1) { return i_v0 - (ValueType)i_v1; }
   friend ValueType operator*(const ValueType& i_v0, const number_property& i_v1) { return i_v0 * (ValueType)i_v1; }
   friend ValueType operator/(const ValueType& i_v0, const number_property& i_v1) { return i_v0 / (ValueType)i_v1; }

   using Property<Host, Accessor, ValueType>::operator =;
   //using PropertyBase<Host, Accessor, ValueType>::operator const ValueType&;	
   //using PropertyBase<Host, Accessor, ValueType>::operator ();
};


template <class host> class quat_accessor : public accessor_base<host, glm::quat>
{
public:
   virtual void mul(host* i_host, const glm::quat& i_value)
   {
      this->set(i_host, this->val * i_value);
   }
};

template<class Host, class Accessor, typename ValueType> class quat_property : public Property<Host, Accessor, ValueType>
{
public:
   quat_property(Host* i_host = nullptr) : Property<Host, Accessor, ValueType>(i_host) {}

   virtual const ValueType& operator *=(const ValueType& i_value)
   {
      this->accessor_inst.mul(this->host_ref, i_value);
      return this->accessor_inst.get(this->host_ref);
   }

   using Property<Host, Accessor, ValueType>::operator =;
};


template <class host> class string_accessor : public accessor_base<host, std::string>
{
public:
   virtual void add(host* i_host, const std::string& i_value)
   {
      this->set(i_host, this->val + i_value);
   }
};

template<class Host, class Accessor, typename ValueType> class string_property : public Property<Host, Accessor, ValueType>
{
public:
   string_property(Host* i_host = nullptr) : Property<Host, Accessor, ValueType>(i_host) {}

   virtual const ValueType& operator +=(const ValueType& i_value)
   {
      this->accessor_inst.add(this->host_ref, i_value);
      return this->accessor_inst.get(this->host_ref);
   }

   using Property<Host, Accessor, ValueType>::operator =;
};


#define def_prop(host, accessor) Property<host, accessor, accessor::value_type>
#define def_tpl_prop(host, accessor) Property<host, accessor<host>, accessor<host>::value_type>
#define def_number_prop(host, accessor) number_property<host, accessor<host>, accessor<host>::value_type>
#define def_quat_prop(host, accessor) quat_property<host, accessor<host>, accessor<host>::value_type>
#define def_mat_prop(host, accessor) number_property<host, accessor<host>, accessor<host>::value_type>
#define def_string_prop(host, accessor) Property<host, accessor<host>, accessor<host>::value_type>
