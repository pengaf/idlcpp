#include "yyfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <set>
#include <string>
#include <algorithm>
#include "ErrorList.h"
#include "SourceFile.h"

#include "TokenNode.h"
#include "IdentifyNode.h"
#include "IdentifyListNode.h"
#include "AttributeNode.h"
#include "AttributeListNode.h"
#include "ScopeNameNode.h"
#include "ScopeNameListNode.h"
#include "EnumeratorNode.h"
#include "EnumeratorListNode.h"
#include "TypeNameNode.h"
#include "TypeNameListNode.h"
#include "FieldNode.h"
#include "PropertyNode.h"
#include "ParameterNode.h"
#include "ParameterListNode.h"
#include "VariableTypeNode.h"
#include "VariableTypeListNode.h"
#include "MethodNode.h"
#include "OperatorNode.h"
#include "TokenListNode.h"
#include "MemberListNode.h"
#include "BaseClassNode.h"
#include "BaseClassListNode.h"
#include "ClassNode.h"
#include "EnumNode.h"
#include "TypedefNode.h"
#include "TypeDeclarationNode.h"
#include "TemplateParameterListNode.h"
#include "TemplateParametersNode.h"
#include "TemplateClassInstanceNode.h"
#include "NamespaceNode.h"
#include "ProgramNode.h"
#include "Compiler.h"

extern "C"
{
extern int yylineno;
extern int yycolumnno;
extern int yytokenno;
extern int yyHasCollectionProperty;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_create_buffer ( FILE *file, int size );
void yy_switch_to_buffer( YY_BUFFER_STATE new_buffer );
void yy_delete_buffer( YY_BUFFER_STATE b );
#define YY_BUF_SIZE 16384
}

std::vector<SyntaxNodeImpl*> g_syntaxNodes;

SyntaxNode* newToken(int nodeType)
{
	TokenNode* res = new TokenNode(nodeType, yytokenno, yylineno, yycolumnno);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newIdentify(const char* str)
{
	IdentifyNode* res = new IdentifyNode(str, yytokenno, yylineno, yycolumnno);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newString(const char* str)
{
	IdentifyNode* res = new IdentifyNode(str, strlen(str) - 1, yytokenno, yylineno, yycolumnno);
	g_syntaxNodes.push_back(res);
	return res;
}

void newCodeBlock(const char* str)
{
	g_compiler.m_currentSourceFile->addEmbededCodeBlock(str, yytokenno);
}

SyntaxNode* newPrimitiveType(SyntaxNode* keyword, PredefinedType type)
{
	assert(snt_keyword_begin_primitive < keyword->m_nodeType && keyword->m_nodeType < snt_keyword_end_primitive);
	TypeNameNode* res = new TypeNameNode((TokenNode*)keyword, type);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newIdentifyList(SyntaxNode* identifyList, SyntaxNode* delimiter, SyntaxNode* identify)
{
	assert(0 == identifyList || snt_identify_list == identifyList->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert((0 == delimiter && 0 == identifyList) || (0 != delimiter && 0 != identifyList));
	assert(snt_identify == identify->m_nodeType);
	IdentifyListNode* res = new IdentifyListNode((IdentifyListNode*)identifyList, (TokenNode*)delimiter, (IdentifyNode*)identify);
	g_syntaxNodes.push_back(res);
	return res;
}


SyntaxNode* newAttribute(SyntaxNode* name, SyntaxNode* content, int u8content)
{
	assert(0 != name && snt_identify == name->m_nodeType);
	assert(0 != content && snt_identify == content->m_nodeType);
	AttributeNode* res = new AttributeNode((IdentifyNode*)name, (IdentifyNode*)content, u8content);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newAttributeList(SyntaxNode* attributeList, SyntaxNode* attribute)
{
	assert(0 == attributeList || snt_attribute_list == attributeList->m_nodeType);
	assert(0 != attribute && snt_attribute == attribute->m_nodeType);
	AttributeListNode* res = new AttributeListNode((AttributeListNode*)attributeList, (AttributeNode*)attribute);
	g_syntaxNodes.push_back(res);
	return res;
}

void setAttributeList(SyntaxNode* member, SyntaxNode* attributeList)
{
	assert(0 != member && 0 != attributeList);
	assert(snt_field == member->m_nodeType
		|| snt_property == member->m_nodeType
		|| snt_method == member->m_nodeType
		|| snt_operator == member->m_nodeType
		|| snt_class == member->m_nodeType
		|| snt_enum == member->m_nodeType
		|| snt_template_class_instance == member->m_nodeType
		|| snt_typedef == member->m_nodeType
		|| snt_type_declaration == member->m_nodeType
		|| snt_namespace == member->m_nodeType);
	assert(snt_attribute_list == attributeList->m_nodeType);

	((MemberNode*)member)->m_attributeList = (AttributeListNode*)attributeList;
}

SyntaxNode* newEnumerator(SyntaxNode* attributeList, SyntaxNode* identify)
{
	assert(0 == attributeList || snt_attribute_list == attributeList->m_nodeType);
	assert(snt_identify == identify->m_nodeType);
	EnumeratorNode* res = new EnumeratorNode((AttributeListNode*)attributeList, (IdentifyNode*)identify);
	g_syntaxNodes.push_back(res);
	return res;
}


SyntaxNode* newEnumeratorList(SyntaxNode* enumeratorList, SyntaxNode* delimiter, SyntaxNode* enumerator)
{
	assert(0 == enumeratorList || snt_enumerator_list == enumeratorList->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert((0 == delimiter && 0 == enumeratorList) || (0 != delimiter && 0 != enumeratorList));
	assert(snt_enumerator == enumerator->m_nodeType);
	EnumeratorListNode* res = new EnumeratorListNode((EnumeratorListNode*)enumeratorList, (TokenNode*)delimiter, (EnumeratorNode*)enumerator);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newEnum(SyntaxNode* keyword, SyntaxNode* keyword2, SyntaxNode* name, SyntaxNode* leftBrace, SyntaxNode* enumeratorList, SyntaxNode* rightBrace)
{
	assert(snt_keyword_enum == keyword->m_nodeType);
	assert(0 == keyword2 || snt_keyword_class == keyword2->m_nodeType);
	assert(snt_identify == name->m_nodeType);
	assert('{' == leftBrace->m_nodeType);
	assert(0 == enumeratorList || snt_enumerator_list == enumeratorList->m_nodeType);
	assert('}' == rightBrace->m_nodeType);
	EnumNode* res = new EnumNode((TokenNode*)keyword, (TokenNode*)keyword2, (IdentifyNode*)name, (TokenNode*)leftBrace, (EnumeratorListNode*)enumeratorList, (TokenNode*)rightBrace);
	g_syntaxNodes.push_back(res);
	return res;
}

void setEnumSemicolon(SyntaxNode* enm, SyntaxNode* semicolon)
{
	assert(snt_enum == enm->m_nodeType);
	assert(';' == semicolon->m_nodeType);
	((EnumNode*)enm)->m_semicolon = (TokenNode*)semicolon;
}

SyntaxNode* newScopeName(SyntaxNode* identify, SyntaxNode* lts, SyntaxNode* parameterList, SyntaxNode* gts)
{
	assert(0 != identify && snt_identify == identify->m_nodeType);
	assert(0 == lts || '<' == lts->m_nodeType);
	assert(0 == parameterList || snt_type_name_list == parameterList->m_nodeType);
	assert(0 == gts || '>' == gts->m_nodeType);
	ScopeNameNode* res = new ScopeNameNode((IdentifyNode*)identify,
		(TokenNode*)lts, (TypeNameListNode*)parameterList, (TokenNode*)gts);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newScopeNameList(SyntaxNode* scopeNameList, SyntaxNode* scopeName)
{
	assert(0 == scopeNameList || snt_scope_name_list == scopeNameList->m_nodeType);
	assert(snt_scope_name == scopeName->m_nodeType);
	ScopeNameListNode* res = new ScopeNameListNode((ScopeNameListNode*)scopeNameList, (ScopeNameNode*)scopeName);
	g_syntaxNodes.push_back(res);
	return res;
}

void setScopeNameListGlobal(SyntaxNode* scopeNameList)
{
	assert(snt_scope_name_list == scopeNameList->m_nodeType);
	static_cast<ScopeNameListNode*>(scopeNameList)->m_global = true;
}

SyntaxNode* newTypeName(SyntaxNode* scopeNameList)
{
	assert(0 != scopeNameList && snt_scope_name_list == scopeNameList->m_nodeType);
	TypeNameNode* res = new TypeNameNode((ScopeNameListNode*)scopeNameList);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newTypeNameList(SyntaxNode* typeNameList, SyntaxNode* delimiter, SyntaxNode* typeName)
{
	assert(0 == typeNameList || snt_type_name_list == typeNameList->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert((0 == delimiter && 0 == typeNameList) || (0 != delimiter && 0 != typeNameList));	
	assert(snt_type_name == typeName->m_nodeType);
	TypeNameListNode* res = new TypeNameListNode((TypeNameListNode*)typeNameList, (TokenNode*)delimiter, (TypeNameNode*)typeName);
	g_syntaxNodes.push_back(res);
	return res;
}

//void setTypeNameFilter(SyntaxNode* syntaxNode, SyntaxNode* filterNode)
//{
//	assert(snt_type_name == syntaxNode->m_nodeType);
//	assert(snt_keyword_nocode == filterNode->m_nodeType
//		|| snt_keyword_nometa == filterNode->m_nodeType);
//
//	((TypeNameNode*)syntaxNode)->m_filterNode = (TokenNode*)filterNode;
//}

void setMemberFilter(SyntaxNode* syntaxNode, SyntaxNode* filterNode)
{
	assert(snt_field == syntaxNode->m_nodeType
		|| snt_property == syntaxNode->m_nodeType
		|| snt_method == syntaxNode->m_nodeType
		|| snt_operator == syntaxNode->m_nodeType
		|| snt_class == syntaxNode->m_nodeType
		|| snt_enum == syntaxNode->m_nodeType
		|| snt_template_class_instance == syntaxNode->m_nodeType
		|| snt_typedef == syntaxNode->m_nodeType
		|| snt_type_declaration == syntaxNode->m_nodeType
		|| snt_namespace == syntaxNode->m_nodeType);
	assert(snt_keyword_nocode == filterNode->m_nodeType
		|| snt_keyword_nometa == filterNode->m_nodeType);

	((MemberNode*)syntaxNode)->m_filterNode = (TokenNode*)filterNode;
}

void setNativeName(SyntaxNode* syntaxNode, SyntaxNode* nativeName)
{
	assert(snt_field == syntaxNode->m_nodeType
		|| snt_method == syntaxNode->m_nodeType
		|| snt_class == syntaxNode->m_nodeType
		|| snt_enum == syntaxNode->m_nodeType
		|| snt_type_declaration == syntaxNode->m_nodeType);
	assert(snt_identify == nativeName->m_nodeType);
	((MemberNode*)syntaxNode)->m_nativeName = (IdentifyNode*)nativeName;
}

SyntaxNode* newVariableType(SyntaxNode* type, TypeCompound typeCompound)
{
	assert(snt_type_name == type->m_nodeType);
	VariableTypeNode* res = new VariableTypeNode((TypeNameNode*)type, typeCompound);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newVariableTypeList(SyntaxNode* variableTypeList, SyntaxNode* delimiter, SyntaxNode* variableType)
{
	assert(0 == variableTypeList || snt_variable_type_list == variableTypeList->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert(snt_variable_type == variableType->m_nodeType);
	VariableTypeListNode* res = new VariableTypeListNode((VariableTypeListNode*)variableTypeList, (TokenNode*)delimiter, (VariableTypeNode*)variableType);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newField(SyntaxNode* variableTypeList, SyntaxNode* name, SyntaxNode* leftBracket, SyntaxNode* rightBracket)
{
	assert(snt_variable_type_list == variableTypeList->m_nodeType && snt_identify == name->m_nodeType);
	assert((0 == leftBracket && 0 == rightBracket) || (leftBracket && rightBracket && '[' == leftBracket->m_nodeType && ']' == rightBracket->m_nodeType));
	VariableTypeNode* variableTypeNode = ((VariableTypeListNode*)variableTypeList)->m_variableType;
	FieldNode* res = new FieldNode(variableTypeNode->m_typeName, variableTypeNode->m_typeCompound, (IdentifyNode*) name, (TokenNode*)leftBracket, (TokenNode*)rightBracket);
	g_syntaxNodes.push_back(res);
	return res;
}

void setFieldStatic(SyntaxNode* syntaxNode, SyntaxNode* stat)
{
	assert(snt_field == syntaxNode->m_nodeType);
	assert(snt_keyword_static == stat->m_nodeType);
	FieldNode* fieldNode = (FieldNode*)syntaxNode;
	fieldNode->m_static = (TokenNode*)stat;
}

void setFieldSemicolon(SyntaxNode* syntaxNode, SyntaxNode* semicolon)
{
	assert(snt_field == syntaxNode->m_nodeType);
	assert(';' == semicolon->m_nodeType);
	FieldNode* fieldNode = (FieldNode*)syntaxNode;
	fieldNode->m_semicolon = (TokenNode*)semicolon;
}

SyntaxNode* newGetter(SyntaxNode* keyword, ParameterPassing passing)
{
	assert(snt_keyword_get == keyword->m_nodeType);
	GetterNode* res = new GetterNode((TokenNode*)keyword, passing);
	g_syntaxNodes.push_back(res);
	return res;
}

void setGetterNativeName(SyntaxNode* syntaxNode, SyntaxNode* nativeName)
{
	assert(snt_getter == syntaxNode->m_nodeType);
	assert(snt_identify == nativeName->m_nodeType);
	GetterNode* getterNode = (GetterNode*)syntaxNode;
	getterNode->m_nativeName = (IdentifyNode*)nativeName;
}

SyntaxNode* newSetter(SyntaxNode* keyword, ParameterPassing passing)
{
	assert(snt_keyword_set == keyword->m_nodeType);
	SetterNode* res = new SetterNode((TokenNode*)keyword, passing);
	g_syntaxNodes.push_back(res);
	return res;
}

void setSetterNativeName(SyntaxNode* syntaxNode, SyntaxNode* nativeName)
{
	assert(snt_setter == syntaxNode->m_nodeType);
	assert(snt_identify == nativeName->m_nodeType);
	SetterNode* setterNode = (SetterNode*)syntaxNode;
	setterNode->m_nativeName = (IdentifyNode*)nativeName;
}

SyntaxNode* newProperty(SyntaxNode* variableTypeList, SyntaxNode* name, PropertyCategory category)
{
	assert(snt_variable_type_list == variableTypeList->m_nodeType && snt_identify == name->m_nodeType);
	VariableTypeNode* variableTypeNode = ((VariableTypeListNode*)variableTypeList)->m_variableType;
	PropertyNode* res = new PropertyNode(variableTypeNode->m_typeName, variableTypeNode->m_typeCompound, (IdentifyNode*)name, category);
	g_syntaxNodes.push_back(res);
	if (collection_property == category)
	{
		yyHasCollectionProperty = 1;
	}
	return res;
}

void setPropertyGetter(SyntaxNode* property, SyntaxNode* getter)
{
	assert(snt_property == property->m_nodeType);
	assert(snt_getter == getter->m_nodeType);
	((PropertyNode*)property)->setGetter((GetterNode*)getter);
}

void setPropertySetter(SyntaxNode* property, SyntaxNode* setter)
{
	assert(snt_property == property->m_nodeType);
	assert(snt_setter == setter->m_nodeType);
	((PropertyNode*)property)->setSetter((SetterNode*)setter);
}

void setPropertyModifier(SyntaxNode* syntaxNode, SyntaxNode* modifier)
{
	assert(snt_property == syntaxNode->m_nodeType);
	assert(snt_keyword_static == modifier->m_nodeType);
	PropertyNode* propertyNode = (PropertyNode*)syntaxNode;
	propertyNode->m_modifier = (TokenNode*)modifier;
}

SyntaxNode* newParameter(SyntaxNode* variableType, ParameterPassing passing, SyntaxNode* name)
{
	assert(snt_variable_type == variableType->m_nodeType && snt_identify == name->m_nodeType);
	ParameterNode* res = new ParameterNode(((VariableTypeNode*)variableType)->m_typeName, ((VariableTypeNode*)variableType)->m_typeCompound, passing, (IdentifyNode*)name);
	g_syntaxNodes.push_back(res);
	return res;
}

void setDefaultParameter(SyntaxNode* parameter, SyntaxNode* defaultDenote)
{
	assert(snt_parameter == parameter->m_nodeType);
	static_cast<ParameterNode*>(parameter)->m_defaultDenote = (TokenNode*)defaultDenote;
}

SyntaxNode* newParameterList(SyntaxNode* parameterList, SyntaxNode* delimiter, SyntaxNode* parameter)
{
	assert(0 == parameterList || snt_parameter_list == parameterList->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert(snt_parameter == parameter->m_nodeType);
	ParameterListNode* res = new ParameterListNode((ParameterListNode*)parameterList, (TokenNode*)delimiter, (ParameterNode*)parameter);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newMethod(SyntaxNode* name, SyntaxNode* leftParenthesis, SyntaxNode* parameterList, SyntaxNode* rightParenthesis, SyntaxNode* constant)
{
	assert(snt_identify == name->m_nodeType);
	assert('(' == leftParenthesis->m_nodeType && ')' == rightParenthesis->m_nodeType);	
	assert(0 == constant || snt_keyword_const == constant->m_nodeType);
	assert(0 == parameterList || snt_parameter_list == parameterList->m_nodeType);
	MethodNode* res = new MethodNode((IdentifyNode*)name, (TokenNode*)leftParenthesis, (ParameterListNode*)parameterList,
		(TokenNode*)rightParenthesis, (TokenNode*)constant);
	g_syntaxNodes.push_back(res);
	return res;
}

void setMethodResult(SyntaxNode* method, SyntaxNode* voidResult, SyntaxNode* variableTypeList)
{
	assert(snt_method == method->m_nodeType);
	assert(0 == voidResult || snt_keyword_void == voidResult->m_nodeType);
	assert(0 == variableTypeList || snt_variable_type_list == variableTypeList->m_nodeType);
	((MethodNode*)method)->m_voidResult = (TokenNode*)voidResult;
	((MethodNode*)method)->m_resultList = (VariableTypeListNode*)variableTypeList;
}

void setMethodModifier(SyntaxNode* method, SyntaxNode* modifier)
{
	assert(snt_method == method->m_nodeType);
	assert(snt_keyword_abstract == modifier->m_nodeType || snt_keyword_virtual == modifier->m_nodeType || snt_keyword_static == modifier->m_nodeType);
	((MethodNode*)method)->m_modifier = (TokenNode*)modifier;
}

void setMethodOverride(SyntaxNode* method)
{
	assert(snt_method == method->m_nodeType);
	((MethodNode*)method)->m_override = true;
}

void setMethodSemicolon(SyntaxNode* syntaxNode, SyntaxNode* semicolon)
{
	assert(snt_method == syntaxNode->m_nodeType);
	assert(';' == semicolon->m_nodeType);
	MethodNode* methodNode = (MethodNode*)syntaxNode;
	methodNode->m_semicolon = (TokenNode*)semicolon;
}

SyntaxNode* newAssignOperator(SyntaxNode* keyword, SyntaxNode* sign, SyntaxNode* leftParenthesis, SyntaxNode* paramTypeName, ParameterPassing paramPassing, SyntaxNode* rightParenthesis, SyntaxNode* semicolon)
{
	assert(snt_keyword_operator == keyword->m_nodeType && '=' == sign->m_nodeType);
	assert('(' == leftParenthesis->m_nodeType && ')' == rightParenthesis->m_nodeType && ';' == semicolon->m_nodeType);
	assert(snt_type_name == paramTypeName->m_nodeType);
	AssignOperatorNode* res = new AssignOperatorNode((TokenNode*)keyword, (TokenNode*)sign, (TokenNode*)leftParenthesis, (TypeNameNode*)paramTypeName, paramPassing,
		(TokenNode*)rightParenthesis, (TokenNode*)semicolon);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newCastOperator(SyntaxNode* keyword, SyntaxNode* resultTypeName, SyntaxNode* leftParenthesis, SyntaxNode* rightParenthesis, SyntaxNode* constant, SyntaxNode* semicolon)
{
	assert(snt_keyword_operator == keyword->m_nodeType);
	assert(snt_type_name == resultTypeName->m_nodeType);
	assert('(' == leftParenthesis->m_nodeType && ')' == rightParenthesis->m_nodeType && ';' == semicolon->m_nodeType);
	assert(snt_keyword_const == constant->m_nodeType);
	CastOperatorNode* res = new CastOperatorNode((TokenNode*)keyword, (TypeNameNode*)resultTypeName, (TokenNode*)leftParenthesis,
		(TokenNode*)rightParenthesis, (TokenNode*)constant, (TokenNode*)semicolon);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newClassMemberList(SyntaxNode* memberList, SyntaxNode* member)
{
	assert(0 == memberList || snt_member_list == memberList->m_nodeType);
	assert(snt_field == member->m_nodeType 
		|| snt_property == member->m_nodeType 
		|| snt_method == member->m_nodeType 
		|| snt_operator == member->m_nodeType
		|| snt_class == member->m_nodeType
		|| snt_enum == member->m_nodeType
		|| snt_typedef == member->m_nodeType 
		|| snt_type_declaration  == member->m_nodeType);
	MemberListNode* res = new MemberListNode((MemberListNode*)memberList, (MemberNode*)member);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newClass(SyntaxNode* keyword, SyntaxNode* category, SyntaxNode* name)
{
	assert(snt_keyword_class == keyword->m_nodeType || snt_keyword_struct == keyword->m_nodeType);
	assert(snt_identify == name->m_nodeType);
	assert(0 == category || snt_identify_list == category->m_nodeType);
	ClassNode* res = new ClassNode((TokenNode*)keyword, (IdentifyListNode*)category, (IdentifyNode*)name);
	g_syntaxNodes.push_back(res);
	return res;	
}

SyntaxNode* newBaseClass(SyntaxNode* typeName, SyntaxNode* filterNode)
{
	assert(snt_type_name == typeName->m_nodeType);
	assert(0 == filterNode || snt_keyword_nocode == filterNode->m_nodeType
		|| snt_keyword_nometa == filterNode->m_nodeType);

	BaseClassNode* res = new BaseClassNode((TypeNameNode*)typeName, (TokenNode*)filterNode);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newBaseClassList(SyntaxNode* baseClassList, SyntaxNode* delimiter, SyntaxNode* baseClass)
{
	assert(0 == baseClassList || snt_base_class_list == baseClassList->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert((0 == delimiter && 0 == baseClassList) || (0 != delimiter && 0 != baseClassList));
	assert(snt_base_class == baseClass->m_nodeType);
	BaseClassListNode* res = new BaseClassListNode((BaseClassListNode*)baseClassList, (TokenNode*)delimiter, (BaseClassNode*)baseClass);
	g_syntaxNodes.push_back(res);
	return res;
}

void setClassBaseList(SyntaxNode* cls, SyntaxNode* colon, SyntaxNode* baseList)
{
	assert(snt_class == cls->m_nodeType);
	assert(0 == colon || ':' == colon->m_nodeType);
	assert(0 == baseList || snt_base_class_list == baseList->m_nodeType);
	((ClassNode*)cls)->m_colon = (TokenNode*)colon;
	((ClassNode*)cls)->m_baseList = (BaseClassListNode*)baseList;
}

void setClassMemberList(SyntaxNode* cls, SyntaxNode* leftBrace, SyntaxNode* memberList, SyntaxNode* rightBrace)
{
	assert(snt_class == cls->m_nodeType);
	assert('{' == leftBrace->m_nodeType);	
	assert(0 == memberList || snt_member_list == memberList->m_nodeType);
	assert('}' == rightBrace->m_nodeType);
	((ClassNode*)cls)->setMemberList((TokenNode*)leftBrace, (MemberListNode*)memberList, (TokenNode*)rightBrace);
}

void setClassSemicolon(SyntaxNode* cls, SyntaxNode* semicolon)
{
	assert(snt_class == cls->m_nodeType);
	assert(';' == semicolon->m_nodeType);
	((ClassNode*)cls)->m_semicolon = (TokenNode*)semicolon;
}

void setClassModifier(SyntaxNode* cls, SyntaxNode* modifier)
{
	assert(snt_class == cls->m_nodeType);
	assert(snt_keyword_abstract == modifier->m_nodeType);
	((ClassNode*)cls)->m_modifier = (TokenNode*)modifier;
}

void setClassOverride(SyntaxNode* cls)
{
	assert(snt_class == cls->m_nodeType);
	((ClassNode*)cls)->m_override = true;
}

void setClassTemplateParameters(SyntaxNode* cls, SyntaxNode* parameters)
{
	assert(snt_class == cls->m_nodeType);
	assert(snt_template_parameters == parameters->m_nodeType);
	((ClassNode*)cls)->setTemplateParameters((TemplateParametersNode*)parameters);
}

SyntaxNode* newTypeDeclaration(SyntaxNode* name, TypeCategory typeCategory)
{
	assert(snt_identify == name->m_nodeType);
	TypeDeclarationNode* res;
	res = new TypeDeclarationNode((IdentifyNode*)name, typeCategory);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newTypedef(SyntaxNode* keyword, SyntaxNode* name, SyntaxNode* typeName)
{
	assert(snt_keyword_typedef == keyword->m_nodeType);
	assert(snt_identify == name->m_nodeType);
	assert(snt_type_name == typeName->m_nodeType);
	TypedefNode* res;
	res = new TypedefNode((TokenNode*)keyword, (IdentifyNode*)name, (TypeNameNode*)typeName);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newTemplateParameterList(SyntaxNode* list, SyntaxNode* delimiter, SyntaxNode* identify)
{
	assert(0 == list || snt_template_parameter_list == list->m_nodeType);
	assert(0 == delimiter || ',' == delimiter->m_nodeType);
	assert(snt_identify == identify->m_nodeType);
	TemplateParameterListNode* res = new TemplateParameterListNode((TemplateParameterListNode*)list, 
		(TokenNode*)delimiter, (IdentifyNode*)identify);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newTemplateParameters(SyntaxNode* keyword, SyntaxNode* lts, SyntaxNode* parameterList, SyntaxNode* gts)
{
	assert(snt_keyword_template == keyword->m_nodeType);
	assert('<' == lts->m_nodeType);
	assert(snt_template_parameter_list == parameterList->m_nodeType);
	assert('>' == gts->m_nodeType);
	TemplateParametersNode* res = new TemplateParametersNode((TokenNode*)keyword,
		(TokenNode*)lts, (TemplateParameterListNode*)parameterList, (TokenNode*)gts);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newTemplateClassInstance(SyntaxNode* name, SyntaxNode* typeNameList)
{
	assert(snt_identify == name->m_nodeType);
	assert(snt_type_name_list == typeNameList->m_nodeType);
	TemplateClassInstanceNode* res = new TemplateClassInstanceNode((IdentifyNode*)name, (TypeNameListNode*)typeNameList);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newTokenList(SyntaxNode* tokenList, SyntaxNode* token)
{
	assert(0 == tokenList || snt_token_list == tokenList->m_nodeType);
	TokenListNode* res = new TokenListNode((TokenListNode*)tokenList, (TokenNode*)token);
	g_syntaxNodes.push_back(res);
	return res;
}

void setTemplateClassInstanceTokenList(SyntaxNode* tci, SyntaxNode* tokenList)
{
	assert(snt_template_class_instance == tci->m_nodeType && snt_token_list == tokenList->m_nodeType);
	((TemplateClassInstanceNode*)tci)->m_tokenList = (TokenListNode*)tokenList;
}

SyntaxNode* newNamespaceMemberList(SyntaxNode* memberList, SyntaxNode* member)
{
	assert(0 == memberList || snt_member_list == memberList->m_nodeType);
	assert(snt_class == member->m_nodeType 
		|| snt_enum == member->m_nodeType
		|| snt_template_class_instance == member->m_nodeType
		|| snt_typedef == member->m_nodeType 
		|| snt_type_declaration == member->m_nodeType
		|| snt_namespace == member->m_nodeType);
	MemberListNode* res = new MemberListNode((MemberListNode*)memberList, (MemberNode*)member);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newNamespace(SyntaxNode* keyword, SyntaxNode* name, SyntaxNode* leftBrace, SyntaxNode* memberList, SyntaxNode* rightBrace)
{
	assert(snt_keyword_namespace == keyword->m_nodeType);
	assert(snt_identify == name->m_nodeType);
	assert('{' == leftBrace->m_nodeType);
	assert(0 == memberList || snt_member_list == memberList->m_nodeType);
	assert('}' == rightBrace->m_nodeType);
	NamespaceNode* res = new NamespaceNode((TokenNode*)keyword, (IdentifyNode*)name, (TokenNode*)leftBrace, (MemberListNode*)memberList, (TokenNode*)rightBrace);
	g_syntaxNodes.push_back(res);
	return res;
}

SyntaxNode* newProgram(SyntaxNode* memberList)
{
	assert(0 == memberList || snt_member_list == memberList->m_nodeType);
	ProgramNode* res = new ProgramNode((MemberListNode*)memberList);
	g_syntaxNodes.push_back(res);
	return res;
}


void invalidString(char* s)
{
	ErrorList_AddItem_CurrentFile(syntax_error_invalid_string, s);
}

void unterminatedCode()
{
	ErrorList_AddItem_CurrentFile(syntax_error_unterminated_code, "unterminated cpp code block, cannot find \"#}\"");
}

void unterminatedComment()
{
	ErrorList_AddItem_CurrentFile(syntax_error_unterminated_comment, "unterminated comment block, cannot find \"*/\"");
}

void attachSyntaxTree(SyntaxNode* tree)
{
	assert(snt_namespace == tree->m_nodeType);
	g_compiler.attachSyntaxTree((ProgramNode*)tree);
}

void freetree()
{
	size_t count = g_syntaxNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		delete g_syntaxNodes[i];
	}
	g_syntaxNodes.clear();
}

void yyerror(char* s)
{
	ErrorList_AddItem_CurrentFile(syntax_error_internal, s);
}
