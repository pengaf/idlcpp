%{
#include "yyfuncs.h"
%}

%union
{
	struct SyntaxNode* sn;
}

%token <sn> ',' ':' ';' '(' ')' '[' ']' '{' '}' '<' '>' '*' '^' '&' '=' '?'
%token <sn> BOOL CHAR WCHAR_T SHORT LONG INT FLOAT DOUBLE SIGNED UNSIGNED
%token <sn> NAMESPACE ENUM CLASS STRUCT STATIC VIRTUAL VOID CONST OPERATOR TYPEDEF TYPENAME
%token <sn> ABSTRACT GET SET NOMETA NOCODE EXPORT OVERRIDE SCOPE IDENTIFY STRING U8STRING TEMPLATE
%type <sn> identifyList enumerator enumeratorList enum_0 enum variableType variableTypeList
%type <sn> field_0 field_1 field getter_0 getter setter_0 setter property_0 property_1 property
%type <sn> parameter_0 parameter parameterList method_0 method_1 method_2 method_3 method
%type <sn> operator_assign operator_cast operator
%type <sn> classMember_0 classMember
%type <sn> primitive scopeName scopeNameList_0 scopeNameList typeName typeNameList classMemberList tokenList
%type <sn> templateParameterList templateParameters templateClassInstance_0 templateClassInstance
%type <sn> class_0 class_1 class_2 class_3 class_4 class_5 class namespaceMember_0 namespaceMember namespaceMemberList namespace program
%type <sn> typeAlias attribute attributeList attributes

%start program

%%

primitive				: BOOL													{$$ = newPrimitiveType($1, pt_bool);}
						| CHAR													{$$ = newPrimitiveType($1, pt_char);}
						| SIGNED CHAR											{$$ = newPrimitiveType($1, pt_schar);}
						| UNSIGNED CHAR											{$$ = newPrimitiveType($1, pt_uchar);}
						| WCHAR_T												{$$ = newPrimitiveType($1, pt_wchar_t);}
						| SHORT													{$$ = newPrimitiveType($1, pt_short);}
						| SHORT INT												{$$ = newPrimitiveType($1, pt_short);}
						| SIGNED SHORT											{$$ = newPrimitiveType($1, pt_short);}
						| SIGNED SHORT INT										{$$ = newPrimitiveType($1, pt_short);}
						| UNSIGNED SHORT										{$$ = newPrimitiveType($1, pt_ushort);}
						| UNSIGNED SHORT INT									{$$ = newPrimitiveType($1, pt_ushort);}
						| LONG													{$$ = newPrimitiveType($1, pt_long);}
						| LONG INT												{$$ = newPrimitiveType($1, pt_long);}
						| SIGNED LONG											{$$ = newPrimitiveType($1, pt_long);}
						| SIGNED LONG INT										{$$ = newPrimitiveType($1, pt_long);}
						| UNSIGNED LONG											{$$ = newPrimitiveType($1, pt_ulong);}
						| UNSIGNED LONG INT										{$$ = newPrimitiveType($1, pt_ulong);}
						| LONG LONG												{$$ = newPrimitiveType($1, pt_longlong);}
						| LONG LONG INT											{$$ = newPrimitiveType($1, pt_longlong);}
						| SIGNED LONG LONG										{$$ = newPrimitiveType($1, pt_longlong);}
						| SIGNED LONG LONG INT									{$$ = newPrimitiveType($1, pt_longlong);}
						| UNSIGNED LONG LONG									{$$ = newPrimitiveType($1, pt_ulonglong);}
						| UNSIGNED LONG LONG INT								{$$ = newPrimitiveType($1, pt_ulonglong);}
						| INT													{$$ = newPrimitiveType($1, pt_int);}
						| SIGNED INT											{$$ = newPrimitiveType($1, pt_int);}
						| SIGNED												{$$ = newPrimitiveType($1, pt_int);}
						| UNSIGNED INT											{$$ = newPrimitiveType($1, pt_uint);}
						| UNSIGNED												{$$ = newPrimitiveType($1, pt_uint);}
						| FLOAT													{$$ = newPrimitiveType($1, pt_float);}
						| DOUBLE												{$$ = newPrimitiveType($1, pt_double);}
						| LONG DOUBLE											{$$ = newPrimitiveType($1, pt_long_double);}
;

attribute				: IDENTIFY												{$$ = newAttribute($1, NULL, 0);}
						| IDENTIFY '=' STRING									{$$ = newAttribute($1, $3, 0);}
						| IDENTIFY '=' U8STRING									{$$ = newAttribute($1, $3, 1);}
;

attributeList			: attribute												{$$ = newAttributeList(NULL, $1);}
						| attributeList ',' attribute							{$$ = newAttributeList($1, $3);}
;

attributes				: '[' attributeList ']'									{$$ = $2;}
						| '[' attributeList ',' ']'								{$$ = $2;}
						| '[' ']'												{$$ = NULL;}
;

identifyList			: IDENTIFY												{$$ = newIdentifyList(NULL, NULL, $1);}
						| identifyList ',' IDENTIFY								{$$ = newIdentifyList($1, $2, $3);}
;

enumerator				: IDENTIFY												{$$ = newEnumerator(NULL, $1);}
						| attributes IDENTIFY									{$$ = newEnumerator($1, $2);}
;

enumeratorList			: enumerator											{$$ = newEnumeratorList(NULL, NULL, $1);}
						| enumeratorList ',' enumerator							{$$ = newEnumeratorList($1, $2, $3);}
;


enum_0					: ENUM IDENTIFY '{' enumeratorList '}'					{$$ = newEnum($1, NULL, $2, $3, $4, $5);}
						| ENUM IDENTIFY '{' enumeratorList ',' '}'				{$$ = newEnum($1, NULL, $2, $3, $4, $6);}
						| ENUM IDENTIFY '{' '}'									{$$ = newEnum($1, NULL, $2, $3, NULL, $4);}
						| ENUM CLASS IDENTIFY '{' enumeratorList '}'			{$$ = newEnum($1, $2, $3, $4, $5, $6);}
						| ENUM CLASS IDENTIFY '{' enumeratorList ',' '}'		{$$ = newEnum($1, $2, $3, $4, $5, $7);}
						| ENUM CLASS IDENTIFY '{' '}'							{$$ = newEnum($1, $2, $3, $4, NULL, $5);}
;

enum					: enum_0 ';'											{$$ = $1; setEnumSemicolon($$, $2);}
						| enum_0 '=' STRING ';'									{$$ = $1; setNativeName($$, $3); setEnumSemicolon($$, $4);}
;

scopeName				: IDENTIFY												{$$ = newScopeName($1, NULL, NULL, NULL);}
						| IDENTIFY '<' typeNameList '>'							{$$ = newScopeName($1, $2, $3, $4);}	
;

scopeNameList_0			: scopeName												{$$ = newScopeNameList(NULL, $1);}
						| scopeNameList_0 SCOPE scopeName						{$$ = newScopeNameList($1, $3);}
;

scopeNameList			: scopeNameList_0										{$$ = $1;}
						| SCOPE scopeNameList_0									{$$ = $2; setScopeNameListGlobal($$);}
;

typeName				: primitive												{$$ = $1;}
						| scopeNameList											{$$ = newTypeName($1);}
;

typeNameList			: typeName												{$$ = newTypeNameList(NULL, NULL, $1);}
						| typeNameList ',' typeName								{$$ = newTypeNameList($1, $2, $3);}
;

typeAlias				: TYPEDEF typeName IDENTIFY ';'							{$$ = newTypedef($1, $3, $2);}
						| TYPENAME IDENTIFY ';'									{$$ = newTypeDeclaration($2, primitive_type);}
						| ENUM IDENTIFY ';'										{$$ = newTypeDeclaration($2, enum_type);}
						| STRUCT IDENTIFY ';'									{$$ = newTypeDeclaration($2, object_type);}
						| CLASS IDENTIFY ';'									{$$ = newTypeDeclaration($2, object_type);}
						| TYPENAME IDENTIFY '=' STRING ';'						{$$ = newTypeDeclaration($2, primitive_type); setNativeName($$, $4);}
						| ENUM IDENTIFY '=' STRING ';'							{$$ = newTypeDeclaration($2, enum_type); setNativeName($$, $4);}
						| STRUCT IDENTIFY '=' STRING ';'						{$$ = newTypeDeclaration($2, object_type); setNativeName($$, $4);}
						| CLASS IDENTIFY '=' STRING ';'							{$$ = newTypeDeclaration($2, object_type); setNativeName($$, $4);}
;

variableType			: typeName												{$$ = newVariableType($1, tc_none);}
						| typeName '*'											{$$ = newVariableType($1, tc_raw_ptr);}
						| typeName '^'											{$$ = newVariableType($1, tc_shared_ptr);}
						| typeName '[' ']' '^' 									{$$ = newVariableType($1, tc_shared_array);}
;

variableTypeList		: variableType											{$$ = newVariableTypeList(NULL, NULL, $1);}
						| variableTypeList ',' variableType						{$$ = newVariableTypeList($1, $2, $3);}
;

field_0					: variableTypeList IDENTIFY								{$$ = newField($1, $2, NULL, NULL);}
						| variableTypeList IDENTIFY '[' ']'						{$$ = newField($1, $2, $3, $4);}
;

field_1					: field_0												{$$ = $1;}
						| STATIC field_0										{$$ = $2; setFieldStatic($$, $1);}
;

field					: field_1 ';'											{$$ = $1; setFieldSemicolon($$, $2);}
						| field_1 '=' STRING ';'								{$$ = $1; setNativeName($$, $3); setFieldSemicolon($$, $4);}
;


getter_0				: GET													{$$ = newGetter($1, pp_value);}
						| GET '&'												{$$ = newGetter($1, pp_reference);}
						| GET CONST '&'											{$$ = newGetter($1, pp_const_reference);}
;

getter					: getter_0												{$$ = $1;}
						| getter_0 '=' STRING									{$$ = $1; setGetterNativeName($$, $3);}
;

setter_0				: SET													{$$ = newSetter($1, pp_value);}
						| SET '&'												{$$ = newSetter($1, pp_reference);}
						| SET CONST '&'											{$$ = newSetter($1, pp_const_reference);}
;

setter					: setter_0												{$$ = $1;}
						| setter_0 '=' STRING									{$$ = $1; setSetterNativeName($$, $3);}
;

property_0				: variableTypeList IDENTIFY								{$$ = newProperty($1, $2, simple_property);}
						| variableTypeList IDENTIFY '[' ']'						{$$ = newProperty($1, $2, array_property);}
						| variableTypeList IDENTIFY '[' '?' ']'					{$$ = newProperty($1, $2, collection_property);}
;

property_1				: property_0 '{' '}' ';'								{$$ = $1;}
						| property_0 '{' getter '}' ';'							{$$ = $1; setPropertyGetter($$, $3);}
						| property_0 '{' setter '}' ';'							{$$ = $1; setPropertySetter($$, $3);}
						| property_0 '{' getter setter '}' ';'					{$$ = $1; setPropertyGetter($$, $3); setPropertySetter($1, $4);}
						| property_0 '{' setter getter '}' ';'					{$$ = $1; setPropertyGetter($$, $4); setPropertySetter($1, $3);}
;

property				: property_1											{$$ = $1;}
						| STATIC property_1										{$$ = $2; setPropertyModifier($$, $1);}
;


parameter_0				: variableType IDENTIFY								{$$ = newParameter($1, pp_value, $2);}
						| variableType '&' IDENTIFY								{$$ = newParameter($1, pp_reference, $3);}
						| variableType CONST '&' IDENTIFY						{$$ = newParameter($1, pp_const_reference, $4);}
;

parameter				: parameter_0											{$$ = $1;}
						| parameter_0 '='										{$$ = $1; setDefaultParameter($$, $2);}

parameterList			: parameter												{$$ = newParameterList(NULL, NULL, $1);}
						| parameterList ',' parameter							{$$ = newParameterList($1, $2, $3);}
;

method_0				: IDENTIFY '(' ')'										{$$ = newMethod($1, $2, NULL, $3, NULL);}
						| IDENTIFY '(' VOID ')'									{$$ = newMethod($1, $2, NULL, $4, NULL);}
						| IDENTIFY '(' parameterList ')'						{$$ = newMethod($1, $2, $3, $4, NULL);}
						| IDENTIFY '(' ')' CONST								{$$ = newMethod($1, $2, NULL, $3, $4);}
						| IDENTIFY '(' VOID ')' CONST							{$$ = newMethod($1, $2, NULL, $4, $5);}
						| IDENTIFY '(' parameterList ')' CONST					{$$ = newMethod($1, $2, $3, $4, $5);}
;

method_1				: method_0												{$$ = $1;}
						| VOID method_0											{$$ = $2; setMethodResult($$, $1, NULL);}
						| variableTypeList method_0								{$$ = $2; setMethodResult($$, NULL, $1);}
						| VOID ',' variableTypeList method_0					{$$ = $4; setMethodResult($$, $1, $3);}
;

method_2				: method_1												{$$ = $1;}
						| ABSTRACT method_1										{$$ = $2; setMethodModifier($$, $1);}
						| VIRTUAL method_1										{$$ = $2; setMethodModifier($$, $1);}
						| STATIC method_1										{$$ = $2; setMethodModifier($$, $1);}
;

method_3				: method_2												{$$ = $1;}
						| OVERRIDE method_2										{$$ = $2; setMethodOverride($$);}
;

method					: method_3 ';'											{$$ = $1; setMethodSemicolon($$, $2);}
						| method_3 '=' STRING ';'								{$$ = $1; setNativeName($$, $3); setMethodSemicolon($$, $4);}
;

operator_assign			: OPERATOR '=' '(' typeName ')' ';'						{$$ = newAssignOperator($1, $2, $3, $4, pp_value, $5, $6);}
						| OPERATOR '=' '(' typeName CONST '&' ')' ';'			{$$ = newAssignOperator($1, $2, $3, $4, pp_const_reference, $7, $8);}
;

operator_cast			: OPERATOR typeName '(' ')' CONST ';'					{$$ = newCastOperator($1, $2, $3, $4, $5, $6);}
;

operator				: operator_assign										{$$ = $1;}
						| operator_cast											{$$ = $1;}
;

classMember_0			: field													{$$ = $1;}
						| property												{$$ = $1;}
						| method												{$$ = $1;}
						| operator												{$$ = $1;}
						| class													{$$ = $1;}
						| enum													{$$ = $1;}
						| typeAlias												{$$ = $1;}
						| NOCODE classMember_0									{$$ = $2; setMemberFilter($$, $1);}
						| NOMETA classMember_0									{$$ = $2; setMemberFilter($$, $1);}
;
			
classMember				: classMember_0											{$$ = $1;}
						| attributes classMember_0								{$$ = $2; setAttributeList($$, $1);}
;

classMemberList			: classMember											{$$ = newClassMemberList(NULL, $1);}
						| ';'													{$$ = NULL;}
						| classMemberList classMember							{$$ = newClassMemberList($1, $2);}
						| classMemberList ';'									{$$ = $1;}
;

templateParameterList	: IDENTIFY												{$$ = newTemplateParameterList(NULL, NULL, $1);}
						| templateParameterList ',' IDENTIFY					{$$ = newTemplateParameterList($1, $2, $3);}
;

templateParameters		: TEMPLATE '<' templateParameterList '>'				{$$ = newTemplateParameters($1, $2, $3, $4);}
;

class_0					: CLASS IDENTIFY										{$$ = newClass($1, NULL, $2);}
						| CLASS '(' identifyList ')' IDENTIFY 					{$$ = newClass($1, $3, $5);}
						| STRUCT IDENTIFY										{$$ = newClass($1, NULL, $2);}
						| STRUCT '(' identifyList ')' IDENTIFY 					{$$ = newClass($1, $3, $5);}
;

class_1					: class_0												{$$ = $1;}
						| class_0 ':' typeNameList								{$$ = $1; setClassBaseList($$, $2, $3);}
;

class_2					: class_1 '{' '}'										{$$ = $1; setClassMemberList($$, $2, NULL, $3);}
						| class_1 '{' classMemberList '}'						{$$ = $1; setClassMemberList($$, $2, $3, $4);}
;

class_3					: class_2 ';'											{$$ = $1; setClassSemicolon($$, $2);}
						| class_2 '=' STRING ';'								{$$ = $1; setNativeName($$, $3); setClassSemicolon($$, $4);}
;

class_4					: class_3												{$$ = $1;}
						| ABSTRACT class_3										{$$ = $2; setClassModifier($$, $1);}
;

class_5					: class_4												{$$ = $1;}
						| OVERRIDE class_4										{$$ = $2; setClassOverride($$);}
;

class					: class_5												{$$ = $1;}
						| templateParameters class_5							{$$ = $2; setClassTemplateParameters($$, $1);}
;


tokenList				: IDENTIFY												{$$ = newTokenList(NULL, $1);}
						| tokenList ',' IDENTIFY								{$$ = newTokenList($1, $3);}
;

templateClassInstance_0	: EXPORT IDENTIFY '<' typeNameList '>'					{$$ = newTemplateClassInstance($2, $4);}
;
						
templateClassInstance	: templateClassInstance_0 ';'							{$$ = $1;}
						| templateClassInstance_0 '{' '}' ';'					{$$ = $1;}
						| templateClassInstance_0 '{' tokenList '}' ';'			{$$ = $1; setTemplateClassInstanceTokenList($1, $3);}
						| templateClassInstance_0 '{' tokenList ',' '}' ';'		{$$ = $1; setTemplateClassInstanceTokenList($1, $3);}
;


namespaceMember_0		: class													{$$ = $1;}
						| enum													{$$ = $1;}
						| templateClassInstance									{$$ = $1;}
						| typeAlias												{$$ = $1;}
						| namespace												{$$ = $1;}
						| NOCODE namespaceMember_0								{$$ = $2; setMemberFilter($$, $1);}
						| NOMETA namespaceMember_0								{$$ = $2; setMemberFilter($$, $1);}
;

namespaceMember			: namespaceMember_0										{$$ = $1;}
						| attributes namespaceMember_0							{$$ = $2; setAttributeList($$, $1);}
;

namespaceMemberList		: namespaceMember										{$$ = newNamespaceMemberList(NULL, $1);}
						| ';'													{$$ = NULL;}
						| namespaceMemberList namespaceMember					{$$ = newNamespaceMemberList($1, $2);}
						| namespaceMemberList ';'								{$$ = $1;}
;

namespace				: NAMESPACE	IDENTIFY '{' '}'							{$$ = newNamespace($1, $2, $3, NULL, $4);}
						| NAMESPACE	IDENTIFY '{' namespaceMemberList '}'		{$$ = newNamespace($1, $2, $3, $4, $5);}
;

program					:														{$$ = newProgram(NULL); attachSyntaxTree($$);}
						| namespaceMemberList									{$$ = newProgram($1); attachSyntaxTree($$);}
;
