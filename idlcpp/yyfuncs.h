#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

struct SyntaxNode
{
	int m_nodeType;
};
typedef struct SyntaxNode SyntaxNode;

enum PredefinedType
{
	pt_bool,
	pt_char,
	pt_schar,
	pt_uchar,
	pt_wchar_t,
	pt_short,
	pt_ushort,
	pt_long,
	pt_ulong,
	pt_longlong,
	pt_ulonglong,
	pt_int,
	pt_uint,
	pt_float,
	pt_double,
	pt_long_double,
	pt_end,
};

typedef enum PredefinedType PredefinedType;

enum TypeCategory
{
	unknown_type,
	type_not_found,
	primitive_type,
	enum_type,
	object_type,
	string_type,
	buffer_type,
	//class_template,
	//template_parameter,
};

typedef enum TypeCategory TypeCategory;

enum TypeCompound
{
	tc_none,
	tc_raw_ptr,
	tc_raw_array,
	tc_shared_ptr,
	tc_shared_array,
};

typedef enum TypeCompound TypeCompound;

enum ParameterPassing
{
	pp_value,
	pp_reference,
	pp_const_reference,
	pp_rvalue_reference,
	pp_const_rvalue_reference,
};

typedef enum ParameterPassing ParameterPassing;

enum SyntaxNodeType
{
	snt_keyword_nometa = 258,
	snt_keyword_nocode,

	snt_keyword_begin_primitive,
	snt_keyword_bool,
	snt_keyword_char,
	snt_keyword_wchar_t,
	snt_keyword_short,
	snt_keyword_long,
	snt_keyword_int,
	snt_keyword_signed,
	snt_keyword_unsigned,
	snt_keyword_float,
	snt_keyword_double,
	snt_keyword_end_primitive,

	snt_begin_output,

	snt_operator_right_reference,
	snt_operator_scope,
	snt_keyword_void,
	snt_keyword_namespace,
	snt_keyword_enum,
	snt_keyword_class,
	snt_keyword_struct,
	snt_keyword_template,
	snt_keyword_abstract,
	snt_keyword_virtual,
	snt_keyword_static,
	snt_keyword_const,
	snt_keyword_operator,
	snt_keyword_get,
	snt_keyword_set,
	snt_keyword_typedef,
	snt_keyword_delegate,

	snt_end_output,

	snt_keyword_typename,
	snt_keyword_export,
	snt_keyword_override,

	snt_identify,
	snt_identify_list,
	snt_enumerator,
	snt_enumerator_list,
	snt_enum,
	snt_scope_name,
	snt_scope_name_list,
	snt_type_name,
	snt_type_name_list,
	snt_parameter,
	snt_parameter_list,
	snt_template_parameter,
	snt_template_parameter_list,
	snt_template_parameters,
	snt_getter,
	snt_setter,
	snt_field,
	snt_property,
	snt_method,
	snt_operator,
	snt_class,
	snt_template_class_instance,
	snt_delegate,
	snt_typedef,
	snt_type_declaration,
	snt_namespace,
	snt_member_list,
	snt_token_list,
	snt_attribute,
	snt_attribute_list,
};

enum PropertyCategory
{
	simple_property,
	array_property,
	collection_property,
};

typedef enum PropertyCategory PropertyCategory;

void newCodeBlock(const char* str);
SyntaxNode* newToken(int nodeType);
SyntaxNode* newIdentify(const char* str);
SyntaxNode* newString(const char* str);

SyntaxNode* newPrimitiveType(SyntaxNode* keyword, PredefinedType type);

SyntaxNode* newIdentifyList(SyntaxNode* identifyList, SyntaxNode* delimiter, SyntaxNode* identify);

SyntaxNode* newAttribute(SyntaxNode* name, SyntaxNode* content, int u8content);
SyntaxNode* newAttributeList(SyntaxNode* attributeList, SyntaxNode* attribute);
void setAttributeList(SyntaxNode* member, SyntaxNode* attributeList);

SyntaxNode* newEnumerator(SyntaxNode* attributeList, SyntaxNode* identify);
SyntaxNode* newEnumeratorList(SyntaxNode* enumeratorList, SyntaxNode* delimiter, SyntaxNode* enumerator);
SyntaxNode* newEnum(SyntaxNode* keyword, SyntaxNode* keyword2, SyntaxNode* name, SyntaxNode* leftBrace, SyntaxNode* enumeratorList, SyntaxNode* rightBrace);
void setEnumSemicolon(SyntaxNode* enm, SyntaxNode* semicolon);

SyntaxNode* newScopeName(SyntaxNode* identify, SyntaxNode* lts, SyntaxNode* parameterList, SyntaxNode* gts);
SyntaxNode* newScopeNameList(SyntaxNode* scopeNameList, SyntaxNode* scopeName);
void setScopeNameListGlobal(SyntaxNode* scopeNameList);
SyntaxNode* newTypeName(SyntaxNode* scopeNameList);
SyntaxNode* newTypeNameList(SyntaxNode* typeNameList, SyntaxNode* delimiter, SyntaxNode* typeName);

void setTypeNameFilter(SyntaxNode* syntaxNode, SyntaxNode* filterNode);
void setMemberFilter(SyntaxNode* syntaxNode, SyntaxNode* filterNode);
void setNativeName(SyntaxNode* syntaxNode, SyntaxNode* nativeName);
SyntaxNode* newField(SyntaxNode* type, TypeCompound typeCompound, SyntaxNode* name);
void setFieldArray(SyntaxNode* syntaxNode, SyntaxNode* leftBracket, SyntaxNode* rightBracket);
void setFieldStatic(SyntaxNode* syntaxNode, SyntaxNode* stat);
void setFieldSemicolon(SyntaxNode* syntaxNode, SyntaxNode* semicolon);

SyntaxNode* newGetter(SyntaxNode* keyword, TypeCompound typeCompound);
void setGetterNativeName(SyntaxNode* syntaxNode, SyntaxNode* nativeName);
void setGetterPassing(SyntaxNode* syntaxNode, ParameterPassing passing);
SyntaxNode* newSetter(SyntaxNode* keyword, TypeCompound typeCompound);
void setSetterNativeName(SyntaxNode* syntaxNode, SyntaxNode* nativeName);
void setSetterPassing(SyntaxNode* syntaxNode, ParameterPassing passing);

SyntaxNode* newProperty(SyntaxNode* type, SyntaxNode* name, PropertyCategory category);
void setPropertyGetter(SyntaxNode* property, SyntaxNode* getter);
void setPropertySetter(SyntaxNode* property, SyntaxNode* setter);
void setPropertyModifier(SyntaxNode* syntaxNode, SyntaxNode* modifier);

SyntaxNode* newParameter(SyntaxNode* type, TypeCompound typeCompound);
void setParameterPassing(SyntaxNode* parameter, ParameterPassing passing);
void setParameterName(SyntaxNode* parameter, SyntaxNode* name);
void setDefaultParameter(SyntaxNode* parameter, SyntaxNode* defaultDenote);

SyntaxNode* newParameterList(SyntaxNode* parameterList, SyntaxNode* delimiter, SyntaxNode* parameter);

SyntaxNode* newMethod(SyntaxNode* name, SyntaxNode* leftParenthesis, SyntaxNode* parameterList, SyntaxNode* rightParenthesis, SyntaxNode* constant);
void setMethodVoidResult(SyntaxNode* method, SyntaxNode* voidResult);
void setMethodResult(SyntaxNode* method, SyntaxNode* result, TypeCompound resultCompound);
void setMethodModifier(SyntaxNode* method, SyntaxNode* modifier);
void setMethodOverride(SyntaxNode* method);
void setMethodSemicolon(SyntaxNode* syntaxNode, SyntaxNode* semicolon);

SyntaxNode* newAssignOperator(SyntaxNode* keyword, SyntaxNode* sign, SyntaxNode* leftParenthesis, SyntaxNode* paramTypeName, ParameterPassing paramPassing, SyntaxNode* rightParenthesis, SyntaxNode* semicolon);
SyntaxNode* newCastOperator(SyntaxNode* keyword, SyntaxNode* resultTypeName, SyntaxNode* leftParenthesis, SyntaxNode* rightParenthesis, SyntaxNode* constant, SyntaxNode* semicolon);

SyntaxNode* newClassMemberList(SyntaxNode* memberList, SyntaxNode* member);
SyntaxNode* newClass(SyntaxNode* keyword, SyntaxNode* category, SyntaxNode* name);
void setClassBaseList(SyntaxNode* cls, SyntaxNode* colon, SyntaxNode* baseList);
void setClassMemberList(SyntaxNode* cls, SyntaxNode* leftBrace, SyntaxNode* memberList, SyntaxNode* rightBrace);
void setClassModifier(SyntaxNode* cls, SyntaxNode* modifier);
void setClassOverride(SyntaxNode* cls);
void setClassTemplateParameters(SyntaxNode* cls, SyntaxNode* parameters);
void setClassSemicolon(SyntaxNode* cls, SyntaxNode* semicolon);

SyntaxNode* newDelegate(SyntaxNode* name, SyntaxNode* leftParenthesis, SyntaxNode* parameterList, SyntaxNode* rightParenthesis, SyntaxNode* semicolon);
void setDelegateResult(SyntaxNode* delegate, SyntaxNode* result, TypeCompound resultTypeCompound);
void setDelegateKeyword(SyntaxNode* delegate, SyntaxNode* keyword);

SyntaxNode* newTypeDeclaration(SyntaxNode* name, TypeCategory typeCategory);
SyntaxNode* newTypedef(SyntaxNode* keyword, SyntaxNode* name, SyntaxNode* typeName);

SyntaxNode* newTemplateParameterList(SyntaxNode* list, SyntaxNode* delimiter, SyntaxNode* identify);
SyntaxNode* newTemplateParameters(SyntaxNode* keyword, SyntaxNode* lts, SyntaxNode* parameterList, SyntaxNode* gts);

SyntaxNode* newTemplateClassInstance(SyntaxNode* name, SyntaxNode* typeNameList);

SyntaxNode* newTokenList(SyntaxNode* tokenList, SyntaxNode* token);
void setTemplateClassInstanceTokenList(SyntaxNode* tci, SyntaxNode* enumeratorList);

SyntaxNode* newNamespaceMemberList(SyntaxNode* memberList, SyntaxNode* member);
SyntaxNode* newNamespace(SyntaxNode* keyword, SyntaxNode* name, SyntaxNode* leftBrace, SyntaxNode* memberList, SyntaxNode* rightBrace);
SyntaxNode* newProgram(SyntaxNode* memberList);

void yyerror(char* s);
void invalidString(char* s);
void unterminatedCode();
void unterminatedComment();
void addSourceFile(const char* fileName);
void attachSyntaxTree(SyntaxNode* tree);

#ifdef __cplusplus
}
#endif
