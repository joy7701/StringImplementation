#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cctype>

char* expand(const char* ptr, int n) // expand into free store
{
	char* p = new char[n]; // allocate memory
	std::strcpy(p, ptr);
	return p;
}

class String {
private:
	char *ptr; // pointer to data
	int sz; // number of characters
	static const int short_max = 15;
	union {
		/* 
			union: all members share the same memory location.
			At any given time a union can contain no more than one object from its list of members
		*/
		
		int space; // unused allocated space
		char ch[short_max + 1]; // leave space for terminating 0
	};
	
	void check(int n) const // range check
	{
		if (n < 0 || sz <= n)
			throw std::out_of_range("String::at()"); 
	}
	
	// ancillary member functions
	void copy_from(const String& x);
	void move_from(String& x);

public:
	String(); // default constructor String{""}

	explicit String(const char* p); // constructor from C-style string String{"Haha"}

	String(const String&); // copy constructor
	String& operator=(const String& x); // copy assignment

	String(String&& x); // move constructor
	String& operator=(String&& x); // move assignment

	~String() { if (short_max < sz) delete[] ptr; } // destructor

	char& operator[](int n) { return ptr[n]; }
	char operator[](int n) const { return ptr[n]; }
	
	char& at(int n) { check(n); return ptr[n]; } // range-checked element access
	char at(int n) const { check(n); return ptr[n]; }
	
	String& operator+(char c); // add c at end
	
	const char* c_str() { return ptr; } // C-style string acccess
	const char* c_str() const { return ptr; }
	
	int size() const { return sz; } // number of elements
	int capacity() const { return (sz <= short_max) ? short_max : sz + space; } // element plus available space
};

void String::copy_from(const String& x) // make *this copy of x
{
	if (x.sz <= short_max) { // working with String on stack
		std::memcpy(this, &x, sizeof(x)); // copy this*; You should use this function  only where there are no objects with built-in types
		ptr = ch;
	}
	else { // working with String on heap
		ptr = expand(x.ptr, x.sz + 1);
		sz = x.sz;
		space = 0;
	}
}

void String::move_from(String& x)
{
	if (x.sz <= short_max) { // working with String on stack
		std::memcpy(this, &x, sizeof(x)); // copy this*; You should use this function  only where there are no objects with built-in types
		ptr = ch;
	}
	else { // grab the elements
		ptr = x.ptr;
		sz = x.sz;
		space = x.space;
		x.ptr = x.ch; // x = "" free info about data in x String
		x.sz = 0;
		x.ch[0] = 0;
	}
}

String::String()
	:sz{0}, ptr{ch} // ptr points to elements, ch is an initial location
{
	ch[0] = 0; // terminating 0
}

String::String(const char* p)
	: sz{static_cast<int>(std::strlen(p))}, ptr{ (sz <= short_max) ? ch : new char[sz + 1]}, space{0}
{
	std::strcpy(ptr, p); // copy characters into ptr from p
}

String::String(const String& x) // copy constructor
{
	copy_from(x); // copy representation from x
}

String::String(String&& x) // move constructor
{
	move_from(x);
}

String& String::operator=(const String& x) // copy assignment
{
	if(this == &x) return *this; // deal with self-assignment
	char* p = (short_max < sz) ? ptr : 0;
	copy_from(x);
	delete[] p; // release free store owned by the target
	return *this;
}

String& String::operator=(String&& x) // move assignment
{
	if(this == &x) return *this; // deal with self-assignment
	if(short_max< sz) delete[] ptr; // delete target
	move_from(x); // does not throw
	return *this;
}

String& String::operator+(char c) // add c at end
{
	if(sz == short_max){ // expand to long string first time
		int n = sz + sz + 2; // double the allocation (+2 because of the terminating 0)
		expand(ptr, n); // reallocate new store
		space = n - sz - 2;
	}
	else if(short_max < sz){ // reallocate other times if space is zero
		if(space == 0){ // expand in free store
			int n = sz + sz + 2; // double the allocation (+2 because of the terminating 0)
			char* p = expand(ptr, n); // reallocate
			delete[] ptr; // delete old pointer
			ptr = p; // assign new pointer to data
			space = n - sz - 2; // free space left in String
		}
		else 
			--space;
	}
	
	ptr[sz] = c; // add c at end
	ptr[++sz] = 0; // increase size and set terminator
	
	return *this;
}

// Helper functions
std::ostream& operator<<(std::ostream& os, const String& s)
{
	return os << s.c_str();
}

std::istream& operator>>(std::istream& is, String& s)
{
	s = String{} ; // clear the target string
	is >> std::ws; // skip whitespace
	char ch = '\0';
	while(is.get(ch) && !std::isspace(ch))
		s = s + ch;
	return is;
}

bool operator==(const String& a, const String& b)
{
	if (a.size() != b.size())
		return false;
	for (int i = 0; i != a.size(); ++i) {
		if(a[i] != b[i])
			return false;
	}
	return true;
}

bool operator!=(const String& a, const String& b)
{
	return !(a == b);
}

const char* begin(const String& x)
{
	return x.c_str();
}

const char* end(const String& x)
{
	return x.c_str() + x.size();
}

// concatenation

String& operator+=(String& a, const String& b)
{
	for (auto x : b)
		a = a + x;
	return a;
}

String operator+(const String& a, const String& b)
{
	String res{a};
	for (auto x: b)
		res = res + x;
	return res;
}

int main()
{
	String s1{"Test"};
	String s2{"abcdsda"};
	
	String res{};
	res = s1 + s2;
	std::cout << res << std::endl;
	return 0;
}