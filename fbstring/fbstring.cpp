#include <string>


class FBSTRING { 
private:
	char *sz_data;
	size_t sz_len;
	size_t sz_alloc;
public:
	FBSTRING();
	FBSTRING(const FBSTRING& str)
	FBSTRING(const FBSTRING& str, size_t pos, size_t n = npos);

}

/* from yetisoft's tinybasic */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
struct FBSTRING {
	FBSTRING();
	FBSTRING(const char *);
	FBSTRING(const FBSTRING&);
	~FBSTRING();
	int operator=(const char*);
	int operator=(const FBSTRING&);
	int operator==(const FBSTRING&);
	int operator!=(const FBSTRING&);
	int operator+=(const FBSTRING&);
	void *data;
	int len;
	int size;
};
typedef int integer;
typedef char zstring;
void print(const FBSTRING&, integer nl);
FBSTRING& concat(const FBSTRING&, const FBSTRING&);
FBSTRING& lcase(const FBSTRING&);
FBSTRING& int_to_str(integer);
FBSTRING& chr(integer);
integer asc(const FBSTRING&);
integer val(const FBSTRING&);
#define exit_ exit
#define EOF_ EOF
#define callocate(n) calloc(n, 1 )
FBSTRING::FBSTRING()
{
	data = 0;
	len = 0;
	size = 0;
}
FBSTRING::FBSTRING(const char *s)
{
	data = 0;
	len = 0;
	size = 0;
	if(s == 0) return;
	int s_len = strlen(s);
	data = malloc(s_len);
	len = s_len;
	size = s_len;
	memcpy(data, s, s_len);
}
FBSTRING::FBSTRING(const FBSTRING& s)
{
	data = 0;
	len = 0;
	size = 0;
	if(s.data == 0) return;
	data = malloc(s.len);
	len = s.len;
	size = s.len;
	memcpy(data, s.data, s.len);
}
FBSTRING::~FBSTRING()
{
	data = 0;
	len = 0;
	size = 0;
}
int FBSTRING::operator=(const char *s)
{
	if(data) free(data);
	data = 0;
	len = 0;
	size = 0;
	if(s == 0) return 0;
	int s_len = strlen(s);
	data = malloc(s_len);
	len = s_len;
	size = s_len;
	memcpy(data, s, s_len);
	return 0;
}
int FBSTRING::operator=(const FBSTRING& s)
{
	data = 0;
	len = 0;
	size = 0;
	if(s.data == 0) return 0;
	data = malloc(s.len);
	len = s.len;
	size = s.len;
	memcpy(data, s.data, s.len);
	return 0;
}
int FBSTRING::operator==(const FBSTRING& s)
{
	if(len != s.len) return 0;
	if((data == 0) && (s.data == 0)) return 1;
	if((data == 0) || (s.data == 0)) return 0;
	if(memcmp(data, s.data, len) != 0) return 0;
	return 1;
}
int FBSTRING::operator!=(const FBSTRING& s)
{
	if(len != s.len) return 1;
	if((data == 0) && (s.data == 0)) return 0;
	if((data == 0) || (s.data == 0)) return 1;
	if(memcmp(data, s.data, len) != 0) return 1;
	return 0;
}
int FBSTRING::operator+=(const FBSTRING& s)
{
	FBSTRING result;
	result.data = data;
	result.len = len;
	result.size = size;
	result = concat(result, s);
	free(data);
	data = result.data;
	len = result.len;
	size = result.size;
	return 0;
}
void print(const FBSTRING& s, integer nl)
{
	int pos;
	for(pos = 0; pos < s.len; pos++) {
		fputc(((char *)s.data)[pos], stdout);
	}
	if(nl) fputc(10, stdout);
}
FBSTRING& concat(const FBSTRING& s1, const FBSTRING& s2)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	result->data = malloc(s1.len + s2.len);
	result->len = s1.len + s2.len;
	result->size = s1.len + s2.len;
	memcpy(result->data, s1.data, s1.len);
	memcpy(&(((char *)result->data)[s1.len]), s2.data, s2.len);
	return *result;
}
FBSTRING& lcase(const FBSTRING& s)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	*result = s;
	int pos;
	for(pos = 0; pos < s.len; pos++) {
		((char *)result->data)[pos] = tolower(((char *)s.data)[pos]);
	}
	return *result;
}
FBSTRING& str_temp(const FBSTRING& s)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	*result = s;
	return *result;
}
FBSTRING& int_to_str(integer i)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	char *tmp = (char *)calloc(32, 1);
	sprintf(tmp, "%i", i);
	int tmp_len = strlen(tmp);
	result->data = tmp;
	result->len = tmp_len;
	result->size = 32;
	return *result;
}
FBSTRING& chr(integer i)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	result->data = malloc(1);
	result->len = 1;
	result->size = 1;
	((char *)result->data)[0] = i;
	return *result;
}
integer val(const FBSTRING& s)
{
	int n;
	char *tmp = (char *)calloc(s.len + 1, 1);
	memcpy(tmp, s.data, s.len);
	n = atoi(tmp);
	free(tmp);
	return n;
}
integer asc(const FBSTRING& s)
{
	if(s.data == 0) return -1;
	return ((char *)s.data)[0];
}
