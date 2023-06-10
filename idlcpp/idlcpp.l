%option noyywrap nodefault yylineno
%x IFILE CODEBLOCK COMMENT

%{
#include <stdlib.h>
#include <string.h>
#include "idlcpp.y.h"
#include "yyfuncs.h"

YY_BUFFER_STATE createBufferState(FILE* file);
int yytokenno = 0;
int yycolumnno = 1;
int yyHasCollectionProperty = 0;
int yyHasDelegate = 0;
%}

%%

"#import"[ \t]*\"	{BEGIN IFILE;}
<IFILE>[^\"\n]+		{
						{
							int c;
							while((c = input()) && c != '\n')
							{}
							addSourceFile(yytext);
							yycolumnno = 0;
							BEGIN INITIAL;
						}
					}
<IFILE>.|\n			{fprintf(stderr, "%d bad import line\n", yylineno); yyterminate();}

"#{"						{yycolumnno += 2; BEGIN CODEBLOCK;}
<CODEBLOCK>"#}"				{yycolumnno += 2; BEGIN INITIAL;}
<CODEBLOCK>[^#\n]+			{newCodeBlock(yytext); yycolumnno += (int)strlen(yytext); yytokenno++;}
<CODEBLOCK>\n				{newCodeBlock(yytext); yycolumnno = 1; yytokenno++;}
<CODEBLOCK>.				{newCodeBlock(yytext); yycolumnno++; yytokenno++;}
<CODEBLOCK><<EOF>>			{unterminatedCode();yyterminate();}

"/*"						{yycolumnno += 2; BEGIN COMMENT;}
<COMMENT>"*/"				{yycolumnno += 2; BEGIN INITIAL;}
<COMMENT>[^*\n]+			{yycolumnno += (int)strlen(yytext); }
<COMMENT>\n					{newCodeBlock("\n"); yycolumnno = 1; }
<COMMENT>.					{yycolumnno++;}
<COMMENT><<EOF>>			{unterminatedComment();yyterminate();}



"//".*\n			{newCodeBlock("\n"); yycolumnno = 1;}

<<EOF>>				{yyterminate();}

\#\#.*				{newCodeBlock(yytext + 2); yycolumnno += (int)strlen(yytext); yytokenno++;}

\#[a-zA-Z_0-9]+		{newCodeBlock(yytext + 1); yycolumnno += (int)strlen(yytext); yytokenno++;}

"*" |
"+" |
"!" |
"^" |
"&" |
"=" |
"<" |
">" |
"{"	|
"}"	|
"["	|
"]"	|
"("	|
")"	|
","	|
";"	|
":"	|
"?"				{yylval.sn = newToken(yytext[0]);							yycolumnno++; yytokenno++; return yytext[0];}

"&&"			{yylval.sn = newToken(snt_operator_right_reference);		yycolumnno += (int)strlen(yytext); yytokenno++; return RVALUE_REFERENCE;}
"::"			{yylval.sn = newToken(snt_operator_scope);					yycolumnno += (int)strlen(yytext); yytokenno++; return SCOPE;}

"bool"			{yylval.sn = newToken(snt_keyword_bool);					yycolumnno += (int)strlen(yytext); yytokenno++; return BOOL;}
"char"			{yylval.sn = newToken(snt_keyword_char);					yycolumnno += (int)strlen(yytext); yytokenno++; return CHAR;}
"wchar_t"		{yylval.sn = newToken(snt_keyword_wchar_t);					yycolumnno += (int)strlen(yytext); yytokenno++; return WCHAR_T;}
"short"			{yylval.sn = newToken(snt_keyword_short);					yycolumnno += (int)strlen(yytext); yytokenno++; return SHORT;}
"long"			{yylval.sn = newToken(snt_keyword_long);					yycolumnno += (int)strlen(yytext); yytokenno++; return LONG;}
"int"			{yylval.sn = newToken(snt_keyword_int);						yycolumnno += (int)strlen(yytext); yytokenno++; return INT;}
"signed"		{yylval.sn = newToken(snt_keyword_signed);					yycolumnno += (int)strlen(yytext); yytokenno++; return SIGNED;}
"unsigned"		{yylval.sn = newToken(snt_keyword_unsigned);				yycolumnno += (int)strlen(yytext); yytokenno++; return UNSIGNED;}
"float"			{yylval.sn = newToken(snt_keyword_float);					yycolumnno += (int)strlen(yytext); yytokenno++; return FLOAT;}
"double"		{yylval.sn = newToken(snt_keyword_double);					yycolumnno += (int)strlen(yytext); yytokenno++; return DOUBLE;}
"void"			{yylval.sn = newToken(snt_keyword_void);					yycolumnno += (int)strlen(yytext); yytokenno++; return VOID;}
"abstract"		{yylval.sn = newToken(snt_keyword_abstract);				yycolumnno += (int)strlen(yytext); yytokenno++; return ABSTRACT;}
"namespace"		{yylval.sn = newToken(snt_keyword_namespace);				yycolumnno += (int)strlen(yytext); yytokenno++; return NAMESPACE;}
"enum"			{yylval.sn = newToken(snt_keyword_enum);					yycolumnno += (int)strlen(yytext); yytokenno++; return ENUM;}
"class"			{yylval.sn = newToken(snt_keyword_class);					yycolumnno += (int)strlen(yytext); yytokenno++; return CLASS;}
"struct"		{yylval.sn = newToken(snt_keyword_struct);					yycolumnno += (int)strlen(yytext); yytokenno++; return STRUCT;}
"virtual"		{yylval.sn = newToken(snt_keyword_virtual);					yycolumnno += (int)strlen(yytext); yytokenno++; return VIRTUAL;}
"static"		{yylval.sn = newToken(snt_keyword_static);					yycolumnno += (int)strlen(yytext); yytokenno++; return STATIC;}
"const"			{yylval.sn = newToken(snt_keyword_const);					yycolumnno += (int)strlen(yytext); yytokenno++; return CONST;}
"operator"		{yylval.sn = newToken(snt_keyword_operator);				yycolumnno += (int)strlen(yytext); yytokenno++; return OPERATOR;}
"typedef"		{yylval.sn = newToken(snt_keyword_typedef);					yycolumnno += (int)strlen(yytext); yytokenno++; return TYPEDEF;}
"typename"		{yylval.sn = newToken(snt_keyword_typename);				yycolumnno += (int)strlen(yytext); yytokenno++; return TYPENAME;}
"get"			{yylval.sn = newToken(snt_keyword_get);						yycolumnno += (int)strlen(yytext); yytokenno++; return GET;}
"set"			{yylval.sn = newToken(snt_keyword_set);						yycolumnno += (int)strlen(yytext); yytokenno++; return SET;}
"nocode"		{yylval.sn = newToken(snt_keyword_nocode);					yycolumnno += (int)strlen(yytext); yytokenno++; return NOCODE;}
"nometa"		{yylval.sn = newToken(snt_keyword_nometa);					yycolumnno += (int)strlen(yytext); yytokenno++; return NOMETA;}
"export"		{yylval.sn = newToken(snt_keyword_export);					yycolumnno += (int)strlen(yytext); yytokenno++; return EXPORT;}
"override"		{yylval.sn = newToken(snt_keyword_override);				yycolumnno += (int)strlen(yytext); yytokenno++; return OVERRIDE;}
"template"		{yylval.sn = newToken(snt_keyword_template);				yycolumnno += (int)strlen(yytext); yytokenno++; return TEMPLATE;}
"delegate"		{yylval.sn = newToken(snt_keyword_delegate);				yycolumnno += (int)strlen(yytext); yytokenno++; return DELEGATE;}


[a-zA-Z_][a-zA-Z_0-9]*	{yylval.sn = newIdentify(yytext);					yycolumnno += (int)strlen(yytext); yytokenno++; return IDENTIFY;}

\"[^\"]*\"				{yylval.sn = newString(yytext + 1);					yycolumnno += (int)strlen(yytext); yytokenno++; return STRING;}
u8\"[^\"]*\"			{yylval.sn = newString(yytext + 3);					yycolumnno += (int)strlen(yytext); yytokenno++; return U8STRING;}

[ \t]							{++yycolumnno;}
"\n"							{newCodeBlock(yytext); yycolumnno = 1;}

[^ ,;:{}\[\]<>()&*^$"/+%^|!~=?\-\t\n]*		{invalidString(yytext); yycolumnno += (int)strlen(yytext);}
.								{invalidString(yytext); yycolumnno += (int)strlen(yytext);}

%%

YY_BUFFER_STATE createBufferState(FILE* file)
{
	YY_BUFFER_STATE bs;
	bs = yy_create_buffer(file, YY_BUF_SIZE);
	return bs;
}