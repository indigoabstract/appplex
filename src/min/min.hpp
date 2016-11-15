#pragma once

#include <stdio.h>
#include <algorithm> 
#include <cctype>
#include <exception>
#include <functional> 
#include <locale>
#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;
using std::weak_ptr;

class ia_sender;
class ia_receiver;


class ia_exception: public std::exception
{
public:
	ia_exception() throw();
	ia_exception(const char* msg) throw();
	ia_exception(std::string msg) throw();
	virtual ~ia_exception() throw();

	// returns a C-style character string describing the general cause of the current error
	virtual const char* what() const throw();

private:
	std::string exmsg;
};


class iadp
{
public:
	virtual ~iadp(){}
	static shared_ptr<iadp> new_instance(std::string iname);

	virtual const std::string& get_name();
	virtual bool is_type(const std::string& itype);
	virtual bool is_processed();
	virtual void process();
	virtual shared_ptr<ia_sender> source();
	virtual shared_ptr<ia_receiver> destination();

protected:
	iadp(const std::string& iname);
	virtual void set_name(const std::string&);

private:
	friend class ia_sender;

	std::string name;
	bool processed;
	weak_ptr<ia_sender> src;
	weak_ptr<ia_receiver> dst;
};


class ia_receiver
{
public:
	ia_receiver(){}
	virtual ~ia_receiver(){}

	virtual void receive(shared_ptr<iadp> idp) = 0;
};


class ia_sender
{
public:
	ia_sender(){}
	virtual ~ia_sender(){}

	virtual void send(shared_ptr<ia_receiver> dst, shared_ptr<iadp> idp);

protected:
	virtual shared_ptr<ia_sender> sender_inst() = 0;
};


class ia_broadcaster : public ia_sender
{
public:
	ia_broadcaster(){}
	virtual ~ia_broadcaster(){}

	virtual void add_receiver(shared_ptr<ia_receiver> ir);
	virtual void remove_receiver(shared_ptr<ia_receiver> ir);

protected:
	virtual void broadcast(shared_ptr<ia_sender> src, shared_ptr<iadp> idp);

	std::vector<weak_ptr<ia_receiver> > receivers;
};


class ia_node : public ia_sender, public ia_receiver
{
public:
	ia_node(){}
	virtual ~ia_node(){}
};

#define int_vect_pass(name) name, sizeof(name) / sizeof(int)


template <class T, class TAl> inline T* begin_ptr(shared_ptr<std::vector<T, TAl> > v){return v->empty() ? 0 : &v->front();}
template <class T, class TAl> inline T* begin_ptr(std::vector<T, TAl>* v){return v->empty() ? 0 : &v->front();}
template <class T, class TAl> inline T* begin_ptr(std::vector<T, TAl>& v){return v.empty() ? 0 : &v.front();}
template <class T, class TAl> inline const T* begin_ptr(const std::vector<T, TAl>& v){return v.empty() ? 0 : &v.front();}

template<typename T> T lerp(const T& start, const T& end, float t)
{
	return start * (1.f - t) + end * t;
}

inline bool is_inside_box(float x, float y, float boxx, float boxy, float boxWidth, float boxHeight)
{
	return (x >= boxx && x < (boxx + boxWidth)) && (y >= boxy && y < (boxy + boxHeight));
}

bool starts_with(const std::string& istr, const std::string& ifind);
bool ends_with(const std::string& istr, const std::string& ifind);

// trim from start
inline std::string ltrim(const std::string& is)
{
	std::string s(is);
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
inline std::string rtrim(const std::string& is)
{
	std::string s(is);
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
inline std::string trim(const std::string& is)
{
	return ltrim(rtrim(is));
}
