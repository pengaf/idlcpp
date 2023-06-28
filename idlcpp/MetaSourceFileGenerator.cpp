#include "MetaSourceFileGenerator.h"
#include "Utility.h"
#include "SourceFile.h"
#include "ProgramNode.h"
#include "NamespaceNode.h"
#include "TokenNode.h"
#include "TokenListNode.h"
#include "IdentifyNode.h"
#include "EnumeratorListNode.h"
#include "EnumeratorNode.h"
#include "EnumNode.h"
#include "ClassNode.h"
#include "TemplateParametersNode.h"
#include "TemplateClassInstanceNode.h"
#include "TypedefNode.h"
#include "typeDeclarationNode.h"
#include "TypeNameListNode.h"
#include "TypeNameNode.h"
#include "MemberListNode.h"
#include "FieldNode.h"
#include "PropertyNode.h"
#include "MethodNode.h"
#include "OperatorNode.h"
#include "ParameterNode.h"
#include "ParameterListNode.h"
#include "VariableTypeNode.h"
#include "VariableTypeListNode.h"
#include "AttributeNode.h"
#include "AttributeListNode.h"
#include "TypeTree.h"
#include "Platform.h"
#include "CommonFuncs.h"
#include "Options.h"
#include "Compiler.h"

#include <assert.h>
#include <algorithm>
#include <vector>
#include <string>

std::string generate_c_string_literal(const std::string& input_string){
	std::string c_string_literal = "\"";
	for (char c : input_string) {
		if (c == '\"') {
			c_string_literal += "\\\"";
		}
		else if (c == '\\') {
			c_string_literal += "\\\\";
		}
		else if (c == '\n') {
			c_string_literal += "\\n";
		}
		else if (c == '\t') {
			c_string_literal += "\\t";
		}
		else if (c == '\r') {
			c_string_literal += "\\r";
		}
		else if (c == '\b') {
			c_string_literal += "\\b";
		}
		else if (c == '\f') {
			c_string_literal += "\\f";
		}
		else {
			c_string_literal += c;
		}
	}
	c_string_literal += "\"";
	return c_string_literal;
}

const char* typeCompoundToString(TypeCompound typeCompound)
{
	switch (typeCompound)
	{
	case tc_raw_ptr:
		return "::paf::TypeCompound::raw_ptr";
	case tc_raw_array:
		return "::paf::TypeCompound::raw_array";
	case tc_shared_ptr:
		return "::paf::TypeCompound::shared_ptr";
	case tc_shared_array:
		return "::paf::TypeCompound::shared_array";
	default:
		return "::paf::TypeCompound::none";
	}
}

const char* parameterPassingToString(ParameterPassing parameterPassing)
{
	switch (parameterPassing)
	{
	case pp_reference:
		return "::paf::Passing::reference";
	case pp_const_reference:
		return "::paf::Passing::const_reference";
	default:
		return "::paf::Passing::value";
	}
}

void writeMetaConstructor(ClassNode* classNode,
	TemplateArguments* templateArguments,
	std::vector<MemberNode*>& nestedTypeNodes,
	std::vector<MemberNode*>& nestedTypeAliasNodes,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	std::vector<MethodNode*>& staticMethodNodes,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	std::vector<MethodNode*>& methodNodes,
	FILE* file, int indentation);

void writeMetaPropertyImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<PropertyNode*>& propertyNodes, FILE* file, int indentation);
void writeMetaMethodImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MethodNode*>& methodNodes, bool isStatic, FILE* file, int indentation);
void writeMetaGetSingletonImpls(MemberNode* typeNode, TemplateArguments* templateArguments, FILE* file, int indentation);
void writeEnumMetaConstructor(EnumNode* enumNode, TemplateArguments* templateArguments, std::vector<EnumeratorNode*>& enumerators, FILE* file, int indentation);

void writeMetaMethodImpl_UseParam(ClassNode* classNode, TemplateArguments* templateArguments, ParameterNode* parameterNode, size_t paramIndex, FILE* file)
{
	char strArg[64];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, templateArguments);

	if (tc_none == parameterNode->m_typeCompound)
	{
		sprintf_s(strArg, "*a%zd", paramIndex);
	}
	else
	{
		sprintf_s(strArg, "a%zd", paramIndex);
	}
	if (0 != paramIndex)
	{
		writeStringToFile(", ", file, 0);
	}
	writeStringToFile(strArg, file, 0);
}

void writeMetaMethodImpl_UseOutputParam(ClassNode* classNode, TemplateArguments* templateArguments, VariableTypeNode* outputNode, size_t paramIndex, bool lastParam, FILE* file)
{
	char strArg[64];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, outputNode->m_typeName, templateArguments);
	sprintf_s(strArg, "res%zd%s", paramIndex, lastParam ? "" : ", ");
	writeStringToFile(strArg, file, 0);
}

void writeMetaMethodImpl_CastSelf(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string className;
	classNode->getNativeName(className, templateArguments);
	sprintf_s(buf, "%s* self;\n", className.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("if(!args[0]->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("return ::paf::ErrorCode::e_invalid_this_type;\n", file, indentation + 1);
	writeStringToFile("}\n", file, indentation);
}

void writeMetaMethodImpl_CastParam(ClassNode* classNode, TemplateArguments* templateArguments, ParameterNode* parameterNode, size_t argIndex, size_t paramIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, templateArguments);
	std::string defaultValue, argNumTest;

	if (parameterNode->m_defaultParamCode)
	{
		defaultValue = " = ";
		defaultValue += parameterNode->m_defaultParamCode->m_code.c_str();
		sprintf_s(buf, "numArgs > %zd && ", argIndex);
		argNumTest = buf;
	}

	if (tc_none == parameterNode->m_typeCompound)
	{
		if (pp_value == parameterNode->m_passing || pp_const_reference == parameterNode->m_passing)
		{
			sprintf_s(buf, "%s val%zd%s, *a%zd;\n", typeName.c_str(), paramIndex, defaultValue.c_str(), paramIndex);
			writeStringToFile(buf, file, indentation + 1);
			sprintf_s(buf, "if(%s!args[%zd]->castToRawPointerOrValue(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &val%zd, (void**)&a%zd))\n", argNumTest.c_str(), argIndex, typeName.c_str(), paramIndex, paramIndex);
			writeStringToFile(buf, file, indentation + 1);
		}
		else
		{
			sprintf_s(buf, "%s* a%zd%s;\n", typeName.c_str(), paramIndex, defaultValue.c_str());
			writeStringToFile(buf, file, indentation + 1);
			sprintf_s(buf, "if(%s!args[%zd]->castToRawPointer(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&a%zd))\n", argNumTest.c_str(), argIndex, typeName.c_str(), paramIndex);
			writeStringToFile(buf, file, indentation + 1);
		}
		//argDeference = "*";
	}
	else if (tc_raw_ptr == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "%s* a%zd%s;\n", typeName.c_str(), paramIndex, defaultValue.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(%s!args[%zd]->castToRawPtr<%s>(a%zd))\n", argNumTest.c_str(), argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_raw_array == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::paf::array_t<%s> a%zd%s;\n", typeName.c_str(), paramIndex, defaultValue.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(%s!args[%zd]->castToRawArray<%s>(a%zd))\n", argNumTest.c_str(), argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_ptr == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::paf::SharedPtr<%s> a%zd%s;\n", typeName.c_str(), paramIndex, defaultValue.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(%s!args[%zd]->castToSharedPtr<%s>(a%zd))\n", argNumTest.c_str(), argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_array == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::paf::SharedArray<%s> a%zd%s;\n", typeName.c_str(), paramIndex, defaultValue.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(%s!args[%zd]->castToSharedArray<%s>(a%zd))\n", argNumTest.c_str(), argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "return ::paf::ErrorCode::e_invalid_arg_type_%zd;\n", paramIndex + 1);
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);
}


void writeMetaMethodImpl_DeclareOutputParam(ClassNode* classNode, TemplateArguments* templateArguments, VariableTypeNode* outputNode, size_t paramIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, outputNode->m_typeName, templateArguments);

	switch (outputNode->m_typeCompound)
	{
	case tc_raw_ptr:
		sprintf_s(buf, "%s* a%zd;\n", typeName.c_str(), paramIndex);
		break;
	case tc_raw_array:
		sprintf_s(buf, "::paf::array_t<%s> res%zd;\n", typeName.c_str(), paramIndex);
		break;
	case tc_shared_ptr:
		sprintf_s(buf, "::paf::SharedPtr<%s> res%zd;\n", typeName.c_str(), paramIndex);
		break;
	case tc_shared_array:
		sprintf_s(buf, "::paf::SharedArray<%s> res%zd;\n", typeName.c_str(), paramIndex);
		break;
	default:
		sprintf_s(buf, "%s res%zd;\n", typeName.c_str(), paramIndex);
	}
	writeStringToFile(buf, file, indentation + 1);
}

void writeMetaMethodImpl_Call(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, std::vector<VariableTypeNode*>& resultsNodes, std::vector<ParameterNode*>& parameterNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	std::string className;
	classNode->getNativeName(className, templateArguments);
	IdentifyNode* methodNameNode = methodNode->m_nativeName ? methodNode->m_nativeName : methodNode->m_name;

	VariableTypeNode* resultNode = nullptr;
	if (!methodNode->m_voidResult && methodNode->m_resultList)
	{
		resultNode = resultsNodes[0];
	}
	if (resultNode)
	{
		switch (resultNode->m_typeCompound)
		{
		case tc_raw_ptr:
			writeStringToFile("results[0].assignRawPtr(", file, indentation);
			break;
		case tc_raw_array:
			writeStringToFile("results[0].assignRawArray(", file, indentation);
			break;
		case tc_shared_ptr:
			writeStringToFile("results[0].assignSharedPtr(", file, indentation);
			break;
		case tc_shared_array:
			writeStringToFile("results[0].assignSharedArray(", file, indentation);
			break;
		default:
			writeStringToFile("results[0].assignValue(", file, indentation);
		}
	}
	else
	{
		writeStringToFile("", file, indentation);
	}

	size_t startOutputParam = methodNode->m_voidResult ? 0 : 1;
	size_t resultCount = resultsNodes.size();
	size_t paramCount = parameterNodes.size();
	bool hasParam = startOutputParam < resultCount || 0 < paramCount;

	if (methodNode->isStatic())
	{
		if (methodNode->m_nativeName)
		{
			sprintf_s(buf, "%s(", methodNameNode->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "%s::%s(", className.c_str(), methodNameNode->m_str.c_str());
		}
	}
	else
	{
		if (methodNameNode->m_str.find(':') != std::string::npos)
		{
			sprintf_s(buf, "%s(self%s", methodNameNode->m_str.c_str(), hasParam ? ", " : "");
		}
		else
		{
			sprintf_s(buf, "self->%s(", methodNameNode->m_str.c_str());
		}
	}
	writeStringToFile(buf, file, 0);

	for (size_t i = startOutputParam; i < resultCount; ++i)
	{
		VariableTypeNode* resultNode = resultsNodes[i];
		bool lastParam = (i + 1) == resultCount && 0 == paramCount;
		writeMetaMethodImpl_UseOutputParam(classNode, templateArguments, resultNode, i, lastParam, file);
	}

	for (size_t i = 0; i < paramCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		writeMetaMethodImpl_UseParam(classNode, templateArguments, parameterNode, i, file);
	}
	if (resultNode)
	{
		writeStringToFile(")", file, 0);
	}
	writeStringToFile(");\n", file, 0);
}

void writeMetaMethodImpl_CastOutputParam(ClassNode* classNode, TemplateArguments* templateArguments, VariableTypeNode* outputNode, size_t paramIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, outputNode->m_typeName, templateArguments);

	switch (outputNode->m_typeCompound)
	{
	case tc_raw_ptr:
		sprintf_s(buf, "results[%zd].assignRawPtr(res%zd);\n", paramIndex, paramIndex);
		break;
	case tc_raw_array:
		sprintf_s(buf, "results[%zd].assignRawArray(res%zd);\n", paramIndex, paramIndex);
		break;
	case tc_shared_ptr:
		sprintf_s(buf, "results[%zd].assignSharedPtr(res%zd);\n", paramIndex, paramIndex);
		break;
	case tc_shared_array:
		sprintf_s(buf, "results[%zd].assignSharedArray(res%zd);\n", paramIndex, paramIndex);
		break;
	default:
		sprintf_s(buf, "results[%zd].assignValue(res%zd);\n", paramIndex, paramIndex);
	}
	writeStringToFile(buf, file, indentation + 1);
}


void MetaSourceFileGenerator::generateCode(FILE* dstFile, SourceFile* sourceFile, const char* fullPathName, const char* baseName)
{
	generateCode_Program(dstFile, sourceFile, fullPathName, baseName);
}

void MetaSourceFileGenerator::generateCode_Program(FILE* file, SourceFile* sourceFile, const char* fileName, const char* cppName)
{
	ProgramNode* programNode = sourceFile->m_syntaxTree;
	char buf[4096];
	std::string pafcorePath;
	GetRelativePath(pafcorePath, fileName, g_options.m_pafcorePath.c_str());
	FormatPathForInclude(pafcorePath);

	writeStringToFile("#pragma once\n\n", file);
	g_compiler.outputUsedTypesForMetaSource(file, sourceFile);
	sprintf_s(buf, "#include \"%s%s\"\n", cppName, g_options.m_metaHeaderFilePostfix.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sNameSpace.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sField.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sProperty.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sMethod.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sEnumType.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sPrimitiveType.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	writeStringToFile("#include <new>\n\n", file);

	writeStringToFile("\nnamespace idlcpp\n{\n\n", file);
	generateCode_Namespace(file, programNode, 1);
	writeStringToFile("}\n\n", file);

	std::vector<TypeNode*> typeNodes;
	CollectTypeNodes(typeNodes, programNode);
	std::reverse(typeNodes.begin(), typeNodes.end());
	size_t typeCount = typeNodes.size();
	for (size_t i = 0; i < typeCount; ++i)
	{
		TypeNode* typeNode = typeNodes[i];
		if (typeNode->getSyntaxNode()->canGenerateMetaCode())
		{
			std::string metaTypeName;
			GetMetaTypeFullName(metaTypeName, typeNode);
			sprintf_s(buf, "AUTO_REGISTER_TYPE(::idlcpp::%s)\n", metaTypeName.c_str());
			writeStringToFile(buf, file);
			//if (typeNode->isTypeDeclaration())
			//{
			//	TypeCategory typeCategory = typeNode->getTypeCategory(0);
			//	const char* typeCategoryName = "";
			//	switch (typeCategory)
			//	{
			//	case enum_type:
			//		typeCategoryName = "enumeration";
			//		break;
			//	case object_type:
			//		typeCategoryName = "object";
			//		break;
			//	case introspectable_type:
			//		typeCategoryName = "introspectable";
			//		break;
			//	case string_type:
			//		typeCategoryName = "string";
			//		break;
			//	case buffer_type:
			//		typeCategoryName = "buffer";
			//		break;
			//	default:
			//		assert(false);
			//	}
			//	std::string typeName;
			//	typeNode->getNativeName(typeName);
			//	sprintf_s(buf, "static_assert(RuntimeTypeOf<%s>::type_category == ::paf::MetaCategory::%s, \"type category error\");\n",
			//		typeName.c_str(), typeCategoryName);
			//	writeStringToFile(buf, file);
			//}
		}
	}
}

void MetaSourceFileGenerator::generateCode_Namespace(FILE* file, NamespaceNode* namespaceNode, int indentation)
{
	if (namespaceNode->isNoMeta())
	{
		return;
	}

	std::vector<MemberNode*> memberNodes;
	namespaceNode->m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(memberNode), 0, indentation);
			break;
		case snt_class:
			if (!memberNode->isTemplateClass())
			{
				generateCode_Class(file, static_cast<ClassNode*>(memberNode), 0, indentation);
			}
			break;
		case snt_namespace:
			generateCode_Namespace(file, static_cast<NamespaceNode*>(memberNode), indentation);
			break;
		case snt_template_class_instance:
			generateCode_TemplateClassInstance(file, static_cast<TemplateClassInstanceNode*>(memberNode), indentation);
			break;
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(memberNode), 0, indentation);
			break;
		case snt_type_declaration:
			generateCode_TypeDeclaration(file, static_cast<TypeDeclarationNode*>(memberNode), 0, indentation);
			break;
		default:
			assert(false);
		}
	}
}

void MetaSourceFileGenerator::generateCode_Enum(FILE* file, EnumNode* enumNode, TemplateArguments* templateArguments, int indentation)
{
	if (enumNode->isNoMeta())
	{
		return;
	}
	std::vector<EnumeratorNode*> enumeratorNodes;
	enumNode->m_enumeratorList->collectEnumeratorNodes(enumeratorNodes);
	std::sort(enumeratorNodes.begin(), enumeratorNodes.end(), CompareEnumeratorPtr());
	writeEnumMetaConstructor(enumNode, templateArguments, enumeratorNodes, file, indentation);
	writeMetaGetSingletonImpls(enumNode, templateArguments, file, indentation);
}

void MetaSourceFileGenerator::generateCode_TemplateClassInstance(FILE* file, TemplateClassInstanceNode* templateClassInstance, int indentation)
{
	if (templateClassInstance->isNoMeta())
	{
		return;
	}
	ClassNode* classNode = static_cast<ClassNode*>(templateClassInstance->m_classTypeNode->m_classNode);
	generateCode_Class(file, classNode, templateClassInstance, indentation);
}

void writePlacementNew_Call(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<ParameterNode*>& parameterNodes, size_t usedParamCount, FILE* file, int indentation)
{
	char buf[4096];
	std::string className;
	classNode->getNativeName(className, templateArguments);
	sprintf_s(buf, "if(%zd == numArgs)\n", usedParamCount);
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "new(address) %s(", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	for (size_t i = 0; i < usedParamCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		writeMetaMethodImpl_UseParam(classNode, templateArguments, parameterNode, i, file);
	}
	writeStringToFile(");\n", file, 0);
	writeStringToFile("return ::paf::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n", file, indentation);
}

void writeOverrideFunction(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* constructor, 
	std::vector<AssignOperatorNode*>& assignOperatorNodes,
	std::vector<CastOperatorNode*>& castOperatorNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);
	std::string className;
	classNode->getNativeName(className, templateArguments);

	//placement new
	sprintf_s(buf, "::paf::ErrorCode %s::construct(void* address, ::paf::Variant** args, uint32_t numArgs)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("if(numArgs == 0)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "return ::paf::DefaultConstructorCaller<%s>::Call(address) ? ::paf::ErrorCode::s_ok : ::paf::ErrorCode::e_not_implemented;\n", className.c_str());
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);
	writeStringToFile("else if(numArgs == 1)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	writeStringToFile("void* other;\n", file, indentation + 2);
	writeStringToFile("if(args[0]->castToRawPointer(this, &other))\n", file, indentation + 2);
	writeStringToFile("{\n", file, indentation + 2);
	sprintf_s(buf, "return ::paf::CopyConstructorCaller<%s>::Call(address, other) ? ::paf::ErrorCode::s_ok : ::paf::ErrorCode::e_not_implemented;\n", className.c_str());
	writeStringToFile(buf, file, indentation + 3);
	writeStringToFile("}\n", file, indentation + 2);
	if (!constructor)
	{
		writeStringToFile("return ::paf::ErrorCode::e_invalid_arg_type_1; \n", file, indentation + 2);
	}
	writeStringToFile("}\n", file, indentation + 1);
	if (constructor)
	{
		std::vector<ParameterNode*> parameterNodes;
		constructor->m_parameterList->collectParameterNodes(parameterNodes);
		size_t paramCount = parameterNodes.size();
		size_t defaultParamCount = constructor->getDefaultParameterCount();

		size_t minParamCount = paramCount - defaultParamCount;
		size_t minArgCount = minParamCount;
		if (0 < minArgCount)
		{
			sprintf_s(buf, "if(numArgs < %zd)\n", minArgCount);
			writeStringToFile(buf, file, indentation + 1);
			writeStringToFile("{\n", file, indentation + 1);
			writeStringToFile("return ::paf::ErrorCode::e_too_few_arguments;\n", file, indentation + 2);
			writeStringToFile("}\n", file, indentation + 1);
		}
		for (size_t i = 0; i < minParamCount; ++i)
		{
			ParameterNode* parameterNode = parameterNodes[i];
			size_t argIndex = i;
			writeMetaMethodImpl_CastParam(classNode, templateArguments, parameterNode, argIndex, i, file, indentation);
		}
		for (size_t i = minParamCount; i < paramCount; ++i)
		{
			ParameterNode* parameterNode = parameterNodes[i];
			size_t argIndex = i;
			writeMetaMethodImpl_CastParam(classNode, templateArguments, parameterNode, argIndex, i, file, indentation);
		}
		writePlacementNew_Call(classNode, templateArguments, parameterNodes, paramCount, file, indentation + 1);
	}
	writeStringToFile("return ::paf::ErrorCode::e_too_many_arguments;\n", file, indentation + 1);

	writeStringToFile("}\n\n", file, indentation);
	sprintf_s(buf, "bool %s::constructArray(void* address, size_t count)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "return ::paf::ArrayConstructorCaller<%s>::Call(address, count);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	sprintf_s(buf, "bool %s::destruct(void* self)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "return ::paf::DestructorCaller<%s>::Call(self);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	sprintf_s(buf, "bool %s::copyAssign(void* self, const void* src)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "return ::paf::CopyAssignmentCaller<%s>::Call(self, src);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	if (!assignOperatorNodes.empty())
	{
		size_t count = assignOperatorNodes.size();
		sprintf_s(buf, "bool %s::assign(void* self, ::paf::Type* srcType, const void* src)\n", metaClassName.c_str());
		writeStringToFile(buf, file, indentation);
		writeStringToFile("{\n", file, indentation);
		for (size_t i = 0; i < count; ++i)
		{
			AssignOperatorNode* assignOperatorNode = assignOperatorNodes[i];
			std::string typeName;
			TypeCategory typeCategory = CalcTypeNativeName(typeName, assignOperatorNode->m_paramTypeName, templateArguments);
			sprintf_s(buf, "if (RuntimeTypeOf<%s>::RuntimeType::GetSingleton() == srcType)\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			writeStringToFile("{\n", file, indentation + 1);
			sprintf_s(buf, "return ::paf::AssignmentCaller<%s, %s>::Call(self, src);\n", className.c_str(), typeName.c_str());
			writeStringToFile(buf, file, indentation + 2);
			writeStringToFile("}\n", file, indentation + 1);
		}
		writeStringToFile("return false;\n", file, indentation + 1);
		writeStringToFile("}\n\n", file, indentation);
	}

	if (!castOperatorNodes.empty())
	{
		size_t count = castOperatorNodes.size();
		sprintf_s(buf, "bool %s::cast(::paf::Type* dstType, void* dst, const void* self)\n", metaClassName.c_str());
		writeStringToFile(buf, file, indentation);
		writeStringToFile("{\n", file, indentation);
		for (size_t i = 0; i < count; ++i)
		{
			CastOperatorNode* castOperatorNode = castOperatorNodes[i];
			std::string typeName;
			TypeCategory typeCategory = CalcTypeNativeName(typeName, castOperatorNode->m_resultTypeName, templateArguments);
			sprintf_s(buf, "if (RuntimeTypeOf<%s>::RuntimeType::GetSingleton() == dstType)\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			writeStringToFile("{\n", file, indentation + 1);
			sprintf_s(buf, "return ::paf::CastCaller<%s, %s>::Call(dst, self);\n", typeName.c_str(), className.c_str());
			writeStringToFile(buf, file, indentation + 2);
			writeStringToFile("}\n", file, indentation + 1);
		}
		writeStringToFile("return false;\n", file, indentation + 1);
		writeStringToFile("}\n\n", file, indentation);
	}

	if (classNode->needSubclassProxy(templateArguments))
	{
		std::string subclassProxyName;
		GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);
		sprintf_s(buf, "::paf::SharedPtr<::paf::Introspectable> %s::createSubclassProxy(::paf::SubclassInvoker* subclassInvoker)\n", metaClassName.c_str());
		writeStringToFile(buf, file, indentation);
		writeStringToFile("{\n", file, indentation);
		sprintf_s(buf, "return ::paf::SharedPtr<%s>::Make(subclassInvoker);\n", subclassProxyName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("}\n\n", file, indentation);
	}
}

struct CompareMethodNode
{
	bool operator()(const MethodNode* m1, const MethodNode* m2) const
	{
		int cmp = m1->m_name->m_str.compare(m2->m_name->m_str);
		if (0 != cmp)
		{
			return cmp < 0;
		}
		return m1->getParameterCount() < m2->getParameterCount();
	}
};

void MetaSourceFileGenerator::generateCode_Class(FILE* file, ClassNode* classNode, TemplateClassInstanceNode* templateClassInstance, int indentation)
{
	if (classNode->isNoMeta())
	{
		return;
	}
	TemplateArguments* templateArguments = templateClassInstance ? &templateClassInstance->m_templateArguments : 0;

	std::vector<IdentifyNode*> reservedNames;
	std::vector<TokenNode*> reservedOperators;
	if (templateClassInstance && templateClassInstance->m_tokenList
		&& templateClassInstance->m_classTypeNode->m_classNode == classNode)
	{
		assert(classNode->m_typeNode == templateClassInstance->m_classTypeNode);
		templateClassInstance->getReservedMembers(reservedNames, reservedOperators);
	}
	bool hasReservedMember = (!reservedNames.empty() || !reservedOperators.empty());

	std::vector<MemberNode*> memberNodes;
	std::vector<MethodNode*> methodNodes;
	std::vector<MethodNode*> staticMethodNodes;
	std::vector<PropertyNode*> propertyNodes;
	std::vector<PropertyNode*> staticPropertyNodes;
	std::vector<FieldNode*> fieldNodes;
	std::vector<FieldNode*> staticFieldNodes;
	std::vector<MemberNode*> nestedTypeNodes;
	std::vector<MemberNode*> nestedTypeAliasNodes;
	std::vector<AssignOperatorNode*> assignOperatorNodes;
	std::vector<CastOperatorNode*> castOperatorNodes;

	MethodNode* constructor = nullptr;

	classNode->m_memberList->collectMemberNodes(memberNodes);
	size_t memberCount = memberNodes.size();
	for (size_t i = 0; i < memberCount; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		if (hasReservedMember)
		{
			if (snt_method == memberNode->m_nodeType || snt_property == memberNode->m_nodeType)
			{
				if (!std::binary_search(reservedNames.begin(), reservedNames.end(), memberNode->m_name, CompareIdentifyPtr()))
				{
					continue;
				}
			}
		}
		if (!memberNode->isNoMeta())
		{
			if (snt_method == memberNode->m_nodeType)
			{
				MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
				if (memberNode->m_name->m_str != classNode->m_name->m_str)
				{
					if (methodNode->isStatic())
					{
						staticMethodNodes.push_back(methodNode);
					}
					else
					{
						methodNodes.push_back(methodNode);
					}
				}
				else
				{
					constructor = methodNode;
				}
			}
			else if (snt_property == memberNode->m_nodeType)
			{
				PropertyNode* propertyNode = static_cast<PropertyNode*>(memberNode);
				if (propertyNode->isStatic())
				{
					propertyNode->m_orderIndex = staticPropertyNodes.size();
					staticPropertyNodes.push_back(propertyNode);
				}
				else
				{
					propertyNode->m_orderIndex = propertyNodes.size();
					propertyNodes.push_back(propertyNode);
				}
			}
			else if (snt_field == memberNode->m_nodeType)
			{
				FieldNode* fieldNode = static_cast<FieldNode*>(memberNode);
				if (fieldNode->isStatic())
				{
					fieldNode->m_orderIndex = staticFieldNodes.size();
					staticFieldNodes.push_back(fieldNode);
				}
				else
				{
					fieldNode->m_orderIndex = fieldNodes.size();
					fieldNodes.push_back(fieldNode);
				}
			}
			else if (snt_enum == memberNode->m_nodeType ||
				snt_class == memberNode->m_nodeType)
			{
				nestedTypeNodes.push_back(memberNode);
			}
			else if (snt_typedef == memberNode->m_nodeType ||
				snt_type_declaration == memberNode->m_nodeType)
			{
				nestedTypeAliasNodes.push_back(memberNode);
			}
			else if (snt_operator == memberNode->m_nodeType)
			{
				switch (static_cast<OperatorNode*>(memberNode)->m_operatorCategory)
				{
				case assgin_operator:
					assignOperatorNodes.push_back(static_cast<AssignOperatorNode*>(memberNode));
					break;
				case cast_operator:
					castOperatorNodes.push_back(static_cast<CastOperatorNode*>(memberNode));
					break;
				}
			}
			else
			{
				assert(false);
			}
		}
	}


	writeMetaConstructor(classNode, templateArguments, nestedTypeNodes, nestedTypeAliasNodes,
		staticFieldNodes, staticPropertyNodes, staticMethodNodes,
		fieldNodes, propertyNodes, methodNodes, file, indentation);


	if (nullptr == classNode->m_category || classNode->m_category->m_str == "object" || classNode->m_category->m_str == "string" || classNode->m_category->m_str == "buffer")
	{
		writeOverrideFunction(classNode, templateArguments, constructor, assignOperatorNodes, castOperatorNodes, file, indentation);
	}
	writeMetaPropertyImpls(classNode, templateArguments, propertyNodes, file, indentation);
	writeMetaPropertyImpls(classNode, templateArguments, staticPropertyNodes, file, indentation);
	writeMetaMethodImpls(classNode, templateArguments, methodNodes, false, file, indentation);
	writeMetaMethodImpls(classNode, templateArguments, staticMethodNodes, true, file, indentation);
	writeMetaGetSingletonImpls(classNode, templateArguments, file, indentation);

	if (classNode->needSubclassProxy(templateArguments))
	{
		generateCode_SubclassProxy(file, classNode, templateArguments, indentation);
	}

	size_t subTypeCount = nestedTypeNodes.size();
	for (size_t i = 0; i < subTypeCount; ++i)
	{
		MemberNode* typeNode = nestedTypeNodes[i];
		switch (typeNode->m_nodeType)
		{
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(typeNode), templateArguments, indentation);
			break;
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(typeNode), templateClassInstance, indentation);
			break;
		default:
			assert(false);
		}
	}

	size_t typeAliasCount = nestedTypeAliasNodes.size();
	for (size_t i = 0; i < typeAliasCount; ++i)
	{
		MemberNode* typeAliasNode = nestedTypeAliasNodes[i];
		switch (typeAliasNode->m_nodeType)
		{
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(typeAliasNode), templateArguments, indentation);
			break;
		case snt_type_declaration:
			generateCode_TypeDeclaration(file, static_cast<TypeDeclarationNode*>(typeAliasNode), templateArguments, indentation);
			break;
		default:
			assert(false);
		}
	}
}

const char* g_metaPropertyImplPrefix = "::paf::ErrorCode ";

const char* g_metaSimplePropertyGetImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Variant* value)\n";
const char* g_metaSimplePropertySetImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Variant* value)\n";

const char* g_metaArrayPropertyGetImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, size_t index, ::paf::Variant* value)\n";
const char* g_metaArrayPropertySetImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, size_t index, ::paf::Variant* value)\n";
const char* g_metaArrayPropertySizeImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, size_t& size)\n";

const char* g_metaCollectionPropertyGetImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Iterator* iterator, ::paf::Variant* value)\n";
const char* g_metaCollectionPropertySetImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Iterator* iterator, size_t removeCount, ::paf::Variant* value)\n";
const char* g_metaCollectionPropertyIterateImplPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::SharedPtr<::paf::Iterator>& iterator)\n";

const char* g_metaStaticSimplePropertyGetImplPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Variant* value)\n";
const char* g_metaStaticSimplePropertySetImplPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Variant* value)\n";

const char* g_metaStaticArrayPropertyGetImplPostfix = "(::paf::StaticProperty* staticProperty, size_t index, ::paf::Variant* value)\n";
const char* g_metaStaticArrayPropertySetImplPostfix = "(::paf::StaticProperty* staticProperty, size_t index, ::paf::Variant* value)\n";
const char* g_metaStaticArrayPropertySizeImplPostfix = "(::paf::StaticProperty* staticProperty, size_t& size)\n";

const char* g_metaStaticCollectionPropertyGetImplPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Iterator* iterator, ::paf::Variant* value)\n";
const char* g_metaStaticCollectionPropertySetImplPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Iterator* iterator, size_t removeCount, ::paf::Variant* value)\n";
const char* g_metaStaticCollectionPropertyIterateImplPostfix = "(::paf::StaticProperty* staticProperty, ::paf::SharedPtr<::paf::Iterator>& iterator)\n";



void writeMetaPropertyGetImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_get_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaStaticSimplePropertyGetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaStaticArrayPropertyGetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaStaticCollectionPropertyGetImplPostfix, file);
			break;
		}
	}
	else
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaSimplePropertyGetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaArrayPropertyGetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaCollectionPropertyGetImplPostfix, file);
			break;
		}
	}
	writeStringToFile("{\n", file, indentation);
	TypeCategory typeCategory = CalcTypeNativeName(typeName, propertyNode->m_typeName, templateArguments);

	if (!propertyNode->isStatic())
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::paf::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}

	switch (propertyNode->m_typeCompound)
	{
	case tc_raw_ptr:
		writeStringToFile("value->assignRawPtr(", file, indentation + 1);
		break;
	case tc_raw_array:
		writeStringToFile("value->assignRawArray(", file, indentation + 1);
		break;
	case tc_shared_ptr:
		writeStringToFile("value->assignSharedPtr(", file, indentation + 1);
		break;
	case tc_shared_array:
		writeStringToFile("value->assignSharedArray(", file, indentation + 1);
		break;
	default:
		writeStringToFile("value->assignValue(", file, indentation + 1);
	}

	const char* strIndex = "";
	if (propertyNode->isArray())
	{
		strIndex = "index";
	}
	else if (propertyNode->isCollection())
	{
		strIndex = "iterator";
	}

	if (propertyNode->isStatic())
	{
		if (propertyNode->m_get->m_nativeName)
		{
			sprintf_s(buf, "%s(%s));\n", propertyNode->m_get->m_nativeName->m_str.c_str(), strIndex);
		}
		else
		{
			sprintf_s(buf, "%s::get_%s(%s));\n", className.c_str(), propertyNode->m_name->m_str.c_str(), strIndex);
		}		
		writeStringToFile(buf, file);
	}
	else
	{
		if (propertyNode->m_get->m_nativeName)
		{
			if (propertyNode->m_get->m_nativeName->m_str.find(':') != std::string::npos)
			{
				sprintf_s(buf, "%s(self%s%s));\n", propertyNode->m_get->m_nativeName->m_str.c_str(),
					propertyNode->isSimple() ? "" : ", ", strIndex);
			}
			else
			{
				sprintf_s(buf, "self->%s(%s));\n", propertyNode->m_get->m_nativeName->m_str.c_str(), strIndex);
			}
		}
		else
		{
			sprintf_s(buf, "self->get_%s(%s));\n", propertyNode->m_name->m_str.c_str(), strIndex);
		}
		writeStringToFile(buf, file);
	}
	writeStringToFile("return ::paf::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertySetImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_set_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaStaticSimplePropertySetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaStaticArrayPropertySetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaStaticCollectionPropertySetImplPostfix, file);
			break;
		}
	}
	else
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaSimplePropertySetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaArrayPropertySetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaCollectionPropertySetImplPostfix, file);
			break;
		}

	}
	writeStringToFile("{\n", file, indentation);

	TypeCategory typeCategory = CalcTypeNativeName(typeName, propertyNode->m_typeName, templateArguments);

	if (!propertyNode->isStatic())
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::paf::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}

	const char* argDeference = "";
	if (tc_none == propertyNode->m_typeCompound)
	{
		if(pp_value == propertyNode->m_set->m_passing || pp_const_reference == propertyNode->m_set->m_passing)
		{
			sprintf_s(buf, "%s argValue, *arg;\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			sprintf_s(buf, "if(!value->castToRawPointerOrValue(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &argValue, (void**)&arg))\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
		}
		else
		{
			sprintf_s(buf, "%s* arg;\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			sprintf_s(buf, "if(!value->castToRawPointer(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&arg))\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
		}
		argDeference = "*";
	}
	else if (tc_raw_ptr == propertyNode->m_typeCompound)
	{
		sprintf_s(buf, "%s* arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToRawPtr<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_raw_array == propertyNode->m_typeCompound)
	{
		sprintf_s(buf, "::paf::array_t<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToRawArray<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_ptr == propertyNode->m_typeCompound)
	{
		sprintf_s(buf, "::paf::SharedPtr<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToSharedPtr<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_array == propertyNode->m_typeCompound)
	{
		sprintf_s(buf, "::paf::SharedArray<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToSharedArray<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("{\n", file, indentation + 1);
	writeStringToFile("return ::paf::ErrorCode::e_invalid_property_type;\n", file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);

	if (propertyNode->isStatic())
	{
		if (propertyNode->m_set->m_nativeName)
		{
			sprintf_s(buf, "%s(", propertyNode->m_set->m_nativeName->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "%s::set_%s(", className.c_str(), propertyNode->m_name->m_str.c_str());
		}
	}
	else
	{
		if (propertyNode->m_set->m_nativeName)
		{
			if (propertyNode->m_set->m_nativeName->m_str.find(':') != std::string::npos)
			{
				sprintf_s(buf, "%s(self, ", propertyNode->m_set->m_nativeName->m_str.c_str());
			}
			else
			{
				sprintf_s(buf, "self->%s(", propertyNode->m_set->m_nativeName->m_str.c_str());
			}
		}
		else
		{
			sprintf_s(buf, "self->set_%s(", propertyNode->m_name->m_str.c_str());
		}
	}
	writeStringToFile(buf, file, indentation + 1);

	const char* otherArgs = "";
	if (propertyNode->isArray())
	{
		otherArgs = "index, ";
	}
	else if (propertyNode->isCollection())
	{
		otherArgs = "iterator, removeCount, ";
	}

	if (!propertyNode->isSimple())
	{
		writeStringToFile("index, ", file);
	}

	sprintf_s(buf, "%s%sarg);\n", otherArgs, argDeference);

	writeStringToFile(buf, file, 0);
	writeStringToFile("return ::paf::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertySizeImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_size_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		writeStringToFile(g_metaStaticArrayPropertySizeImplPostfix, file);
	}
	else
	{
		writeStringToFile(g_metaArrayPropertySizeImplPostfix, file);
	}
	writeStringToFile("{\n", file, indentation);

	if (propertyNode->isStatic())
	{
		sprintf_s(buf, "size = %s::size_%s();\n", className.c_str(), propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::paf::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
		
		sprintf_s(buf, "size = self->size_%s();\n", propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("return ::paf::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertyIteratorImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_iterate_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		writeStringToFile(g_metaStaticCollectionPropertyIterateImplPostfix, file);
	}
	else
	{
		writeStringToFile(g_metaCollectionPropertyIterateImplPostfix, file);
	}
	writeStringToFile("{\n", file, indentation);

	if (propertyNode->isStatic())
	{
		sprintf_s(buf, "iterator = %s::iterate_%s();\n", className.c_str(), propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::paf::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);

		sprintf_s(buf, "iterator = self->iterate_%s();\n", propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("return ::paf::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertyImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<PropertyNode*>& propertyNodes, FILE* file, int indentation)
{
	size_t propertyCount = propertyNodes.size();
	if (0 < propertyCount)
	{
		for (size_t i = 0; i < propertyCount; ++i)
		{
			PropertyNode* propertyNode = propertyNodes[i];
			if (0 != propertyNode->m_get)
			{
				writeMetaPropertyGetImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
			if (0 != propertyNode->m_set)
			{
				writeMetaPropertySetImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
			if (propertyNode->isArray())
			{
				writeMetaPropertySizeImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
			else if (propertyNode->isCollection())
			{
				writeMetaPropertyIteratorImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
		}
	}
}

const char g_metaMethodImplPrefix[] = "::paf::ErrorCode ";
const char g_metaMethodImplPostfix[] = "(::paf::Variant* results, uint32_t& numResults, ::paf::Variant** args, uint32_t numArgs)\n";


void writeMetaMethodImpl(
	ClassNode* classNode, 
	TemplateArguments* templateArguments,
	MethodNode* methodNode,
	bool isStatic, 
	size_t methodIndex, 
	FILE* file, 
	int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaMethodImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), methodNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	writeStringToFile(g_metaMethodImplPostfix, file);
	writeStringToFile("{\n", file, indentation);

	std::vector<VariableTypeNode*> resultNodes;
	if (methodNode->m_resultList)
	{
		methodNode->m_resultList->collectVariableTypeNodes(resultNodes);
	}
	size_t resultCount = resultNodes.size();

	if (0 < resultCount)
	{
		sprintf_s(buf, "if(numResults < %zd)\n", resultCount);
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::paf::ErrorCode::e_too_few_results;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}

	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t paramCount = parameterNodes.size();
	size_t defaultParamCount = methodNode->getDefaultParameterCount();

	size_t minParamCount = paramCount - defaultParamCount;
	size_t minArgCount = minParamCount + (isStatic ? 0 : 1);
	size_t maxArgCount = paramCount + (isStatic ? 0 : 1);
	if (0 < minArgCount)
	{
		sprintf_s(buf, "if(numArgs < %zd)\n", minArgCount);
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::paf::ErrorCode::e_too_few_arguments;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}
	sprintf_s(buf, "if(numArgs > %zd)\n", maxArgCount);
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	writeStringToFile("return ::paf::ErrorCode::e_too_many_arguments;\n", file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);

	if (!isStatic)
	{
		writeMetaMethodImpl_CastSelf(classNode, templateArguments, methodNode, file, indentation + 1);
	}
	for (size_t i = 0; i < minParamCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		size_t argIndex = isStatic ? i : i + 1;
		writeMetaMethodImpl_CastParam(classNode, templateArguments, parameterNode, argIndex, i, file, indentation);
	}
	for (size_t i = minParamCount; i < paramCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		size_t argIndex = isStatic ? i : i + 1;
		writeMetaMethodImpl_CastParam(classNode, templateArguments, parameterNode, argIndex, i, file, indentation);
	}

	size_t startOutputParam = methodNode->m_voidResult ? 0 : 1;
	for (size_t i = startOutputParam; i < resultCount; ++i)
	{
		writeMetaMethodImpl_DeclareOutputParam(classNode, templateArguments, resultNodes[i], i, file, indentation);
	}
	
	writeMetaMethodImpl_Call(classNode, templateArguments, methodNode, resultNodes, parameterNodes, isStatic, file, indentation + 1);
	
	for (size_t i = startOutputParam; i < resultCount; ++i)
	{
		writeMetaMethodImpl_CastOutputParam(classNode, templateArguments, resultNodes[i], i, file, indentation);
	}
	sprintf_s(buf, "numResults = %zd;\n", resultCount);
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("return ::paf::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaMethodImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MethodNode*>& methodNodes, bool isStatic, FILE* file, int indentation)
{
	size_t methodCount = methodNodes.size();
	if (0 < methodCount)
	{
		for (size_t i = 0; i < methodCount; ++i)
		{
			MethodNode* methodNode = methodNodes[i];
			writeMetaMethodImpl(classNode, templateArguments, methodNode, isStatic, i, file, indentation);
		}
	}
}

void writeMetaConstructor_attributesForType(std::map<SyntaxNodeImpl*, size_t> attributesOffsets, SyntaxNodeImpl* memberNode, FILE* file, int indentation)
{
	char buf[4096];
	auto it = attributesOffsets.find(memberNode);
	if (it != attributesOffsets.end())
	{
		sprintf_s(buf, "m_attributes = &s_attributeses[%zd];\n", it->second);
		writeStringToFile(buf, file, indentation);
	}
}

struct AttributeOffsetAndCount
{
	SyntaxNodeImpl* node;
	size_t offset;
	size_t count;
};

template<typename T>
void writeMetaConstructor_Attributes_Member(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	T* memberNode, FILE* file, int indentation)
{
	AttributeListNode* attributeList = memberNode->m_attributeList;
	if (attributeList)
	{
		std::vector<AttributeNode*> attributeNodes;
		attributeList->collectAttributeNodes(attributeNodes);
		std::sort(attributeNodes.begin(), attributeNodes.end(), CompareAttributePtr());
		size_t count = attributeNodes.size();
		for (size_t i = 0; i < count; ++i)
		{
			AttributeNode* attributeNode = attributeNodes[i];
			writeStringToFile("{ \"", file, indentation);
			writeStringToFile(attributeNode->m_name->m_str.c_str(), file);
			if (attributeNode->m_u8content)
			{
				writeStringToFile("\", u8\"", file);
			}
			else
			{
				writeStringToFile("\", \"", file);
			}
			if (attributeNode->m_content)
			{
				writeStringToFile(attributeNode->m_content->m_str.c_str(), file);
			}
			writeStringToFile("\" },\n", file);
		}
		AttributeOffsetAndCount aoac;
		aoac.node = memberNode;
		aoac.offset = offset;
		aoac.count = count;
		attributeOffsetAndCounts.push_back(aoac);
		offset += count;
	}
}

void writeMetaConstructor_Attributes_Method(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	std::vector<MethodNode*>::iterator first, std::vector<MethodNode*>::iterator last, FILE* file, int indentation)
{
	std::vector<AttributeNode*> attributeNodes;
	for (auto it = first; it != last; ++it)
	{
		MethodNode* methodNode = *it;
		AttributeListNode* attributeList = methodNode->m_attributeList;
		if (attributeList)
		{
			attributeList->collectAttributeNodes(attributeNodes);
		}
	}
	if (!attributeNodes.empty())
	{
		std::sort(attributeNodes.begin(), attributeNodes.end(), CompareAttributePtr());
		size_t count = attributeNodes.size();
		for (size_t i = 0; i < count; ++i)
		{
			AttributeNode* attributeNode = attributeNodes[i];
			writeStringToFile("{ \"", file, indentation);
			writeStringToFile(attributeNode->m_name->m_str.c_str(), file);
			writeStringToFile("\", u8\"", file);
			if (attributeNode->m_content)
			{
				writeStringToFile(attributeNode->m_content->m_str.c_str(), file);
			}
			writeStringToFile("\" },\n", file);
		}
		AttributeOffsetAndCount aoac;
		aoac.node = *first;
		aoac.offset = offset;
		aoac.count = count;
		attributeOffsetAndCounts.push_back(aoac);
		offset += count;
	}
}

template<typename T>
void writeMetaConstructor_Attributes_Members(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	std::vector<T*>& memberNodes, FILE* file, int indentation)
{
	size_t count = memberNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		T* node = memberNodes[i];
		writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, node, file, indentation);
	}
}

void writeMetaConstructor_Attributes_Methods(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	std::vector<MethodNode*>& methodNodes, FILE* file, int indentation)
{
	std::vector<MethodNode*>::iterator begin = methodNodes.begin();
	std::vector<MethodNode*>::iterator end = methodNodes.end();
	std::vector<MethodNode*>::iterator first = begin;
	std::vector<MethodNode*>::iterator last = begin;
	for (; first != end;)
	{
		++last;
		if (last == end || (*last)->m_name->m_str != (*first)->m_name->m_str)
		{
			writeMetaConstructor_Attributes_Method(attributeOffsetAndCounts, offset, first, last, file, indentation);
			first = last;
		}
	}
}

template<typename T>
bool noAttributes(T& t)
{
	size_t count = t.size();
	for (size_t i = 0; i < count; ++i)
	{
		AttributeListNode* node = t[i]->m_attributeList;
		if (node)
		{
			return false;
		}
	}
	return true;
}


void writeMetaConstructor_Attributeses(std::map<SyntaxNodeImpl*, size_t>& attributessOffsets,
	ClassNode* classNode,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	std::vector<MethodNode*>& staticMethodNodes,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	std::vector<MethodNode*>& methodNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	if (0 == classNode->m_attributeList
		&& noAttributes(staticFieldNodes)
		&& noAttributes(staticPropertyNodes)
		&& noAttributes(staticMethodNodes)
		&& noAttributes(fieldNodes)
		&& noAttributes(propertyNodes)
		&& noAttributes(methodNodes))
	{
		return;
	}

	size_t offset = 0;
	std::vector<AttributeOffsetAndCount> attributeOffsetAndCounts;

	writeStringToFile("static ::paf::Attribute s_attributes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, classNode, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, staticFieldNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, staticPropertyNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Methods(attributeOffsetAndCounts, offset, staticMethodNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, fieldNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, propertyNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Methods(attributeOffsetAndCounts, offset, methodNodes, file, indentation + 1);
	writeStringToFile("};\n", file, indentation);

	size_t size = attributeOffsetAndCounts.size();

	writeStringToFile("static ::paf::Attributes s_attributeses[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < size; ++i)
	{
		AttributeOffsetAndCount& aoac = attributeOffsetAndCounts[i];
		sprintf_s(buf, "{ %zd, &s_attributes[%zd] },\n", aoac.count, aoac.offset);
		writeStringToFile(buf, file, indentation + 1);
		attributessOffsets.insert(std::make_pair(aoac.node, i));
	}
	writeStringToFile("};\n", file, indentation);
}

void writeEnumMetaConstructor_Attributeses(std::map<SyntaxNodeImpl*, size_t>& attributessOffsets,
	EnumNode* enumNode, std::vector<EnumeratorNode*>& enumeratorNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	if (0 == enumNode->m_attributeList
		&& noAttributes(enumeratorNodes))
	{
		return;
	}
	size_t offset = 0;
	std::vector<AttributeOffsetAndCount> attributeOffsetAndCounts;
	writeStringToFile("static ::paf::Attribute s_attributes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, enumNode, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, enumeratorNodes, file, indentation + 1);
	writeStringToFile("};\n", file, indentation);

	size_t size = attributeOffsetAndCounts.size();

	writeStringToFile("static ::paf::Attributes s_attributeses[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < size; ++i)
	{
		AttributeOffsetAndCount& aoac = attributeOffsetAndCounts[i];
		sprintf_s(buf, "{ %zd, &s_attributes[%zd] },\n", aoac.count, aoac.offset);
		writeStringToFile(buf, file, indentation + 1);
		attributessOffsets.insert(std::make_pair(aoac.node, i));
	}
	writeStringToFile("};\n", file, indentation);
}

void writeTypeDefMetaConstructor_Attributeses(std::map<SyntaxNodeImpl*, size_t>& attributessOffsets,
	MemberNode* memberNode, FILE* file, int indentation)
{
	char buf[4096];
	if (0 == memberNode->m_attributeList)
	{
		return;
	}
	size_t offset = 0;
	std::vector<AttributeOffsetAndCount> attributeOffsetAndCounts;
	writeStringToFile("static ::paf::Attribute s_attributes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, memberNode, file, indentation + 1);
	writeStringToFile("};\n", file, indentation);

	size_t size = attributeOffsetAndCounts.size();

	writeStringToFile("static ::paf::Attributes s_attributeses[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < size; ++i)
	{
		AttributeOffsetAndCount& aoac = attributeOffsetAndCounts[i];
		sprintf_s(buf, "{ %zd, &s_attributes[%zd] },\n", aoac.count, aoac.offset);
		writeStringToFile(buf, file, indentation + 1);
		attributessOffsets.insert(std::make_pair(aoac.node, i));
	}
	writeStringToFile("};\n", file, indentation);
}

void writeMetaConstructor_Fields(ClassNode* classNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t>& attributesOffsets,
	std::vector<FieldNode*>& fieldNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	char strAttributes[256];
	if (fieldNodes.empty())
	{
		return;
	}
	std::string className;
	classNode->getNativeName(className, templateArguments);
	size_t count = fieldNodes.size();

	if (isStatic)
	{
		writeStringToFile("static ::paf::StaticField s_staticFields[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::paf::InstanceField s_instanceFields[] = \n", file, indentation);
	}
	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		FieldNode* fieldNode = fieldNodes[i];

		if (fieldNode->m_attributeList)
		{
			auto it = attributesOffsets.find(fieldNode);
			assert(it != attributesOffsets.end());
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		IdentifyNode* fieldNameNode = fieldNode->m_nativeName ? fieldNode->m_nativeName : fieldNode->m_name;
		char arraySize[512];
		if (fieldNode->isArray())
		{
			sprintf_s(arraySize, "paf_field_array_size_of(%s, %s)", className.c_str(), fieldNameNode->m_str.c_str());
		}
		else
		{
			strcpy_s(arraySize, "0");
		}
		std::string typeName;
		TypeCategory typeCategory = CalcTypeNativeName(typeName, fieldNode->m_typeName, templateArguments);
		const char* typeCompound = typeCompoundToString(fieldNode->m_typeCompound);

		if (isStatic)
		{
			sprintf_s(buf, "::paf::StaticField(\"%s\", %s, RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s, %s, (size_t)&%s::%s),\n",
				fieldNode->m_name->m_str.c_str(), strAttributes, typeName.c_str(), typeCompound, arraySize, className.c_str(), fieldNameNode->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "::paf::InstanceField(\"%s\", %s, GetSingleton(), RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s, %s, offsetof(%s, %s)),\n",
				fieldNode->m_name->m_str.c_str(), strAttributes, typeName.c_str(), typeCompound, arraySize, className.c_str(), fieldNameNode->m_str.c_str());
		}

		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	if (isStatic)
	{
		writeStringToFile("m_staticFields = s_staticFields;\n", file, indentation);
		writeStringToFile("m_staticFieldCount = paf_array_size_of(s_staticFields);\n", file, indentation);
	}
	else
	{
		writeStringToFile("m_instanceFields = s_instanceFields;\n", file, indentation);
		writeStringToFile("m_instanceFieldCount = paf_array_size_of(s_instanceFields);\n", file, indentation);
	}
}

void writeMetaConstructor_Properties(ClassNode* classNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t> attributesOffsets,
	std::vector<PropertyNode*>& propertyNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	char strAttributes[256];
	char getterFunc[256];
	char setterFunc[256];
	char valueType[256];

	if (propertyNodes.empty())
	{
		return;
	}
	size_t count = propertyNodes.size();

	if (isStatic)
	{
		writeStringToFile("static ::paf::StaticProperty s_staticProperties[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::paf::InstanceProperty s_instanceProperties[] = \n", file, indentation);
	}

	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		PropertyNode* propertyNode = propertyNodes[i];
		if (propertyNode->m_attributeList)
		{
			auto it = attributesOffsets.find(propertyNode);
			assert(it != attributesOffsets.end());
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		std::string typeName;
		TypeCategory typeCategory = CalcTypeNativeName(typeName, propertyNode->m_typeName, templateArguments);
		sprintf_s(valueType, "RuntimeTypeOf<%s>::RuntimeType::GetSingleton()", typeName.c_str());

		const char* typeCompound = typeCompoundToString(propertyNode->m_typeCompound);
		if (propertyNode->m_get)
		{
			sprintf_s(getterFunc, "%s_get_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
		}
		else
		{
			strcpy_s(getterFunc, "nullptr");
		}
		if (propertyNode->m_set)
		{
			sprintf_s(setterFunc, "%s_set_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
		}
		else
		{
			strcpy_s(setterFunc, "nullptr");
		}

		if (propertyNode->isArray())
		{
			char sizeFunc[256];
			sprintf_s(sizeFunc, "%s_size_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());

			if (isStatic)
			{
				sprintf_s(buf, "::paf::StaticProperty(\"%s\", %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					typeCompound, sizeFunc, getterFunc, setterFunc);
			}
			else
			{
				sprintf_s(buf, "::paf::InstanceProperty(\"%s\", %s, GetSingleton(), %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					typeCompound, sizeFunc, getterFunc, setterFunc);
			}
		}
		else if (propertyNode->isCollection())
		{
			char iterateFunc[256];

			sprintf_s(iterateFunc, "%s_iterate_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());

			if (isStatic)
			{
				sprintf_s(buf, "::paf::StaticProperty(\"%s\", %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					typeCompound, iterateFunc, getterFunc, setterFunc);
			}
			else
			{
				sprintf_s(buf, "::paf::InstanceProperty(\"%s\", %s, GetSingleton(), %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					typeCompound, iterateFunc, getterFunc, setterFunc);
			}
		}
		else
		{
			assert(propertyNode->isSimple());
			if (isStatic)
			{
				sprintf_s(buf, "::paf::StaticProperty(\"%s\", %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					typeCompound, getterFunc, setterFunc);
			}
			else
			{
				sprintf_s(buf, "::paf::InstanceProperty(\"%s\", %s, GetSingleton(), %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType, 
					typeCompound, getterFunc, setterFunc);
			}
		}
		writeStringToFile(buf, file, indentation + 1);
	}

	writeStringToFile("};\n", file, indentation);

	if (isStatic)
	{
		writeStringToFile("m_staticProperties = s_staticProperties;\n", file, indentation);
		writeStringToFile("m_staticPropertyCount = paf_array_size_of(s_staticProperties);\n", file, indentation);
	}
	else
	{
		writeStringToFile("m_instanceProperties = s_instanceProperties;\n", file, indentation);
		writeStringToFile("m_instancePropertyCount = paf_array_size_of(s_instanceProperties);\n", file, indentation);
	}
}

void writeMetaConstructor_Method_Result(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, size_t index, FILE* file, int indentation)
{
	char buf[4096];

	std::vector<VariableTypeNode*> resultNodes;
	methodNode->m_resultList->collectVariableTypeNodes(resultNodes);
	for (VariableTypeNode* resultNode : resultNodes)
	{
		std::string typeName;
		TypeCategory typeCategory = CalcTypeNativeName(typeName, resultNode->m_typeName, templateArguments);
		const char* typeCompound = typeCompoundToString(resultNode->m_typeCompound);
		sprintf_s(buf, "::paf::MethodResult(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s),\n",
			typeName.c_str(), typeCompound);
		writeStringToFile(buf, file, indentation);
	}
}

void writeMetaConstructor_Method_Arguments(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, size_t index, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;

	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	assert(parameterNodes.size() == methodNode->getParameterCount());

	for (ParameterNode* parameterNode : parameterNodes)
	{
		const char* passing = parameterPassingToString(parameterNode->m_passing);
		const char* typeCompound = typeCompoundToString(parameterNode->m_typeCompound);
		TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, templateArguments);
		sprintf_s(buf, "::paf::MethodArgument(\"%s\", RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s, %s),\n",
			parameterNode->m_name->m_str.c_str(), typeName.c_str(), typeCompound, passing);
		writeStringToFile(buf, file, indentation);
	}
}

void writeMetaConstructor_Method_DefaultArguments(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, size_t index, FILE* file, int indentation)
{
	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectDefaultParameterNodes(parameterNodes);
	assert(parameterNodes.size() == methodNode->getDefaultParameterCount());
	for (ParameterNode* parameterNode : parameterNodes)
	{
		std::string strLiteral = generate_c_string_literal(parameterNode->m_defaultParamCode->m_code);
		strLiteral += ",\n";
		writeStringToFile(strLiteral.c_str(), file, indentation);
	}
}

void writeMetaConstructor_Methods(ClassNode* classNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t> attributesOffsets,
	std::vector<MethodNode*>& methodNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];

	if (methodNodes.empty())
	{
		return;
	}

	size_t count = methodNodes.size();
	
	//Results
	bool hasResults = false;
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		size_t resultCount = methodNode->getResultCount();
		if (resultCount > 0)
		{
			hasResults = true;
			break;
		}
	}

	if (hasResults)
	{
		if (isStatic)
		{
			writeStringToFile("static ::paf::MethodResult s_staticResults[] = \n", file, indentation);
		}
		else
		{
			writeStringToFile("static ::paf::MethodResult s_instanceResults[] = \n", file, indentation);
		}
		writeStringToFile("{\n", file, indentation);
		for (size_t i = 0; i < count; ++i)
		{
			MethodNode* methodNode = methodNodes[i];
			writeMetaConstructor_Method_Result(classNode, templateArguments, methodNode, i, file, indentation + 1);
		}
		writeStringToFile("};\n", file, indentation);
	}

	//Arguments
	bool hasArguments = false;
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		size_t paramCount = methodNode->getParameterCount();
		if (paramCount > 0)
		{
			hasArguments = true;
			break;
		}
	}

	if (hasArguments)
	{
		if (isStatic)
		{
			writeStringToFile("static ::paf::MethodArgument s_staticArguments[] = \n", file, indentation);
		}
		else
		{
			writeStringToFile("static ::paf::MethodArgument s_instanceArguments[] = \n", file, indentation);
		}
		writeStringToFile("{\n", file, indentation);
		for (size_t i = 0; i < count; ++i)
		{
			MethodNode* methodNode = methodNodes[i];
			writeMetaConstructor_Method_Arguments(classNode, templateArguments, methodNode, i, file, indentation + 1);
		}
		writeStringToFile("};\n", file, indentation);
	}

	//Arguments
	bool hasDefaultArguments = false;
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		size_t defaultParamCount = methodNode->getDefaultParameterCount();
		if (defaultParamCount > 0)
		{
			hasDefaultArguments = true;
			break;
		}
	}

	if (hasDefaultArguments)
	{
		if (isStatic)
		{
			writeStringToFile("static const char* s_staticDefaultArguments[] = \n", file, indentation);
		}
		else
		{
			writeStringToFile("static const char* s_instanceDefaultArguments[] = \n", file, indentation);
		}
		writeStringToFile("{\n", file, indentation);
		for (size_t i = 0; i < count; ++i)
		{
			MethodNode* methodNode = methodNodes[i];
			writeMetaConstructor_Method_DefaultArguments(classNode, templateArguments, methodNode, i, file, indentation + 1);
		}
		writeStringToFile("};\n", file, indentation);
	}

	//Method
	if (isStatic)
	{
		writeStringToFile("static ::paf::StaticMethod s_staticMethods[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::paf::InstanceMethod s_instanceMethods[] = \n", file, indentation);
	}
	writeStringToFile("{\n", file, indentation);

	size_t resultOffset = 0;
	size_t argumentOffset = 0;
	size_t defaultArgumentOffset = 0;
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		size_t resultCount = methodNode->getResultCount();
		size_t parameterCount = methodNode->getParameterCount();
		size_t defaultParameterCount = methodNode->getDefaultParameterCount();

		char strAttributes[256];
		const char* methodName = methodNode->m_name->m_str.c_str();
		auto it = attributesOffsets.find(methodNode);
		if (it != attributesOffsets.end())
		{
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		char strResults[256];
		char strOutputArguments[256];
		if (!methodNode->m_voidResult)
		{
			assert(resultCount > 0);
			sprintf_s(strResults, "&%s[%zd]", isStatic ? "s_staticResults" : "s_instanceResults", resultOffset);
			if (resultCount > 1)
			{
				sprintf_s(strOutputArguments, "&%s[%zd]", isStatic ? "s_staticResults" : "s_instanceResults", resultOffset + 1);
			}
			else
			{
				strcpy_s(strOutputArguments, "nullptr");
			}
		}
		else
		{
			strcpy_s(strResults, "nullptr");
			if (resultCount > 0)
			{
				sprintf_s(strOutputArguments, "&%s[%zd]", isStatic ? "s_staticResults" : "s_instanceResults", resultOffset);
			}
			else
			{
				strcpy_s(strOutputArguments, "nullptr");
			}
		}
		resultOffset += resultCount;

		char strArguments[256];
		if (parameterCount > 0)
		{
			sprintf_s(strArguments, "&%s[%zd]", isStatic ? "s_staticArguments" : "s_instanceArguments", argumentOffset);
		}
		else
		{
			strcpy_s(strArguments, "nullptr");
		}

		char strDefaultArguments[256];
		if (defaultParameterCount > 0)
		{
			sprintf_s(strDefaultArguments, "&%s[%zd]", isStatic ? "s_staticDefaultArguments" : "s_instanceDefaultArguments", defaultArgumentOffset);
		}
		else
		{
			strcpy_s(strDefaultArguments, "nullptr");
		}

		sprintf_s(buf, "::paf::%s(\"%s\", %s, %s_%s, %s, %s, %zd, %s, %zd, %s, %zd),\n",
			isStatic ? "StaticMethod" : "InstanceMethod",
			methodName, strAttributes, classNode->m_name->m_str.c_str(), methodName,
			strResults, strOutputArguments, methodNode->m_voidResult ? resultCount : resultCount - 1,
			strArguments, parameterCount, strDefaultArguments, defaultParameterCount);
		writeStringToFile(buf, file, indentation + 1);
		argumentOffset += parameterCount;
		defaultArgumentOffset += defaultParameterCount;
	}
	writeStringToFile("};\n", file, indentation);

	if (isStatic)
	{
		writeStringToFile("m_staticMethods = s_staticMethods;\n", file, indentation);
		writeStringToFile("m_staticMethodCount = paf_array_size_of(s_staticMethods);\n", file, indentation);
	}
	else
	{
		writeStringToFile("m_instanceMethods = s_instanceMethods;\n", file, indentation);
		writeStringToFile("m_instanceMethodCount = paf_array_size_of(s_instanceMethods);\n", file, indentation);
	}
}

bool MethodNodeNameEqual(MethodNode* arg1, MethodNode* arg2)
{
	return arg1->m_name->m_str == arg2->m_name->m_str;
}

void writeMetaConstructor_Member(
	std::vector<MemberNode*>& nestedTypeNodes,
	std::vector<MemberNode*>& nestedTypeAliasNodes,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	const std::vector<MethodNode*>& staticMethodNodes_,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	const std::vector<MethodNode*>& methodNodes_,
	FILE* file, int indentation)
{
	char buf[4096];

	std::vector<MethodNode*> staticMethodNodes = staticMethodNodes_;
	std::vector<MethodNode*> methodNodes = methodNodes_;

	auto it = std::unique(staticMethodNodes.begin(), staticMethodNodes.end(), MethodNodeNameEqual);
	staticMethodNodes.erase(it, staticMethodNodes.end());

	it = std::unique(methodNodes.begin(), methodNodes.end(), MethodNodeNameEqual);
	methodNodes.erase(it, methodNodes.end());

	if (nestedTypeNodes.empty() && nestedTypeAliasNodes.empty() &&
		staticFieldNodes.empty() && staticPropertyNodes.empty() && staticMethodNodes.empty()
		&& fieldNodes.empty() && propertyNodes.empty() && methodNodes.empty())
	{
		return;
	}


	writeStringToFile("static Metadata* s_members[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	size_t nestedTypeCount = nestedTypeNodes.size();
	size_t nestedTypeAliasCount = nestedTypeAliasNodes.size();
	size_t staticFieldCount = staticFieldNodes.size();
	size_t staticPropertyCount = staticPropertyNodes.size();
	size_t staticMethodCount = staticMethodNodes.size();
	size_t fieldCount = fieldNodes.size();
	size_t propertyCount = propertyNodes.size();
	size_t methodCount = methodNodes.size();

	size_t currentNestedType = 0;
	size_t currentNestedTypeAlias = 0;
	size_t currentStaticField = 0;
	size_t currentStaticProperty = 0;
	size_t currentStaticMethod = 0;
	size_t currentField = 0;
	size_t currentProperty = 0;
	size_t currentMethod = 0;

	enum MemberCategory
	{
		unknown_member,
		nested_type,
		nested_type_alias,
		static_field,
		static_property,
		static_method,
		instance_field,
		instance_property,
		instance_method,
	};
	while (true)
	{
		MemberNode* current = 0;
		MemberCategory category = unknown_member;
		if (currentNestedType < nestedTypeCount)
		{
			current = nestedTypeNodes[currentNestedType];
			category = nested_type;
		}
		if (currentNestedTypeAlias < nestedTypeAliasCount)
		{
			MemberNode* memberNode = nestedTypeAliasNodes[currentNestedTypeAlias];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = nested_type_alias;
			}
		}
		if (currentStaticField < staticFieldCount)
		{
			MemberNode* memberNode = staticFieldNodes[currentStaticField];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = static_field;
			}
		}
		if (currentStaticProperty < staticPropertyCount)
		{
			MemberNode* memberNode = staticPropertyNodes[currentStaticProperty];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = static_property;
			}
		}
		if (currentStaticMethod < staticMethodCount)
		{
			MemberNode* memberNode = staticMethodNodes[currentStaticMethod];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = static_method;
			}
		}
		if (currentField < fieldCount)
		{
			MemberNode* memberNode = fieldNodes[currentField];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = instance_field;
			}
		}
		if (currentProperty < propertyCount)
		{
			MemberNode* memberNode = propertyNodes[currentProperty];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = instance_property;
			}
		}
		if (currentMethod < methodCount)
		{
			MemberNode* memberNode = methodNodes[currentMethod];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = instance_method;
			}
		}
		if (unknown_member == category)
		{
			break;
		}
		switch (category)
		{
		case nested_type:
			sprintf_s(buf, "s_nestedTypes[%zd],\n", currentNestedType);
			++currentNestedType;
			break;
		case nested_type_alias:
			sprintf_s(buf, "s_nestedTypeAliases[%zd],\n", currentNestedTypeAlias);
			++currentNestedTypeAlias;
			break;
		case static_field:
			sprintf_s(buf, "&s_staticFields[%zd],\n", staticFieldNodes[currentStaticField]->m_orderIndex);
			++currentStaticField;
			break;
		case static_property:
			sprintf_s(buf, "&s_staticProperties[%zd],\n", staticPropertyNodes[currentStaticProperty]->m_orderIndex);
			++currentStaticProperty;
			break;
		case static_method:
			sprintf_s(buf, "&s_staticMethods[%zd],\n", currentStaticMethod);
			++currentStaticMethod;
			break;
		case instance_field:
			sprintf_s(buf, "&s_instanceFields[%zd],\n", fieldNodes[currentField]->m_orderIndex);
			++currentField;
			break;
		case instance_property:
			sprintf_s(buf, "&s_instanceProperties[%zd],\n", propertyNodes[currentProperty]->m_orderIndex);
			++currentProperty;
			break;
		case instance_method:
			sprintf_s(buf, "&s_instanceMethods[%zd],\n", currentMethod);
			++currentMethod;
			break;
		default:
			assert(false);
		}
		writeStringToFile(buf, file, indentation + 1);
	}

	writeStringToFile("};\n", file, indentation);
	writeStringToFile("m_members = s_members;\n", file, indentation);
	writeStringToFile("m_memberCount = paf_array_size_of(s_members);\n", file, indentation);

}

void writeMetaConstructor_BaseClasses(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::vector<std::pair<TokenNode*, TypeNameNode*>> tempNodes;
	classNode->m_baseList->collectTypeNameNodesNotNoMeta(tempNodes);
	std::vector<TypeNameNode*> typeNameNodes;
	size_t count = tempNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = tempNodes[i].second;
		TypeNode* typeNode = typeNameNode->getActualTypeNode(templateArguments);
		if (!typeNode->getSyntaxNode()->isNoMeta())
		{
			typeNameNodes.push_back(typeNameNode);
		}
	}

	if (typeNameNodes.empty())
	{
		return;
	}
	std::string className;
	classNode->getNativeName(className, templateArguments);

	writeStringToFile("static BaseClass s_baseClasses[] =\n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	
	count = typeNameNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = typeNameNodes[i];

		TypeCategory typeCategory = CalcTypeNativeName(typeName, typeNameNode, templateArguments);
			sprintf_s(buf, "{RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), paf_base_offset_of(%s, %s)},\n",
				typeName.c_str(), className.c_str(), typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);	
	}
	writeStringToFile("};\n", file, indentation);
	writeStringToFile("m_baseClasses = s_baseClasses;\n", file, indentation);
	writeStringToFile("m_baseClassCount = paf_array_size_of(s_baseClasses);\n", file, indentation);
}

void writeMetaConstructor_ClassTypeIterators(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::vector<std::pair<TokenNode*, TypeNameNode*>> tempNodes;
	classNode->m_baseList->collectTypeNameNodesNotNoMeta(tempNodes);
	std::vector<TypeNameNode*> typeNameNodes;
	size_t count = tempNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = tempNodes[i].second;
		TypeNode* typeNode = typeNameNode->getActualTypeNode(templateArguments);
		if (!typeNode->getSyntaxNode()->isNoMeta())
		{
			typeNameNodes.push_back(typeNameNode);
		}
	}

	if (typeNameNodes.empty())
	{
		return;
	}
	//std::string className;
	//classNode->getNativeName(className, templateArguments);

	writeStringToFile("static ::paf::ClassTypeIterator s_classTypeIterators[] =\n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	count = typeNameNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = typeNameNodes[i];
		TypeCategory typeCategory = CalcTypeNativeName(typeName, typeNameNode, templateArguments);
		sprintf_s(buf, "::paf::ClassTypeIterator(RuntimeTypeOf<%s>::RuntimeType::GetSingleton()->m_firstDerivedClass, this),\n",
			typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = typeNameNodes[i];
		TypeCategory typeCategory = CalcTypeNativeName(typeName, typeNameNode, templateArguments);
		sprintf_s(buf, "RuntimeTypeOf<%s>::RuntimeType::GetSingleton()->m_firstDerivedClass = &s_classTypeIterators[%zd];\n",
			typeName.c_str(), i);
		writeStringToFile(buf, file, indentation);
	}
	writeStringToFile("m_classTypeIterators = s_classTypeIterators;\n", file, indentation);
}

void writeMetaConstructor_NestedTypes(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MemberNode*>& nestedTypeNodes, FILE* file, int indentation)
{
	char buf[4096];
	if (nestedTypeNodes.empty())
	{
		return;
	}
	size_t count = nestedTypeNodes.size();
	writeStringToFile("static ::paf::Type* s_nestedTypes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* typeNode = nestedTypeNodes[i];
		std::string metaTypeName;
		GetMetaTypeFullName(metaTypeName, typeNode, templateArguments);

		sprintf_s(buf, "%s::GetSingleton(),\n", metaTypeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	writeStringToFile("m_nestedTypes = s_nestedTypes;\n", file, indentation);
	writeStringToFile("m_nestedTypeCount = paf_array_size_of(s_nestedTypes);\n", file, indentation);
}

void writeMetaConstructor_NestedTypeAliases(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MemberNode*>& nestedTypeAliasNodes, FILE* file, int indentation)
{
	char buf[4096];
	if (nestedTypeAliasNodes.empty())
	{
		return;
	}
	size_t count = nestedTypeAliasNodes.size();
	writeStringToFile("static ::paf::TypeAlias* s_nestedTypeAliases[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* typeAliasNode = nestedTypeAliasNodes[i];
		std::string metaTypeName;
		GetMetaTypeFullName(metaTypeName, typeAliasNode, templateArguments);
		sprintf_s(buf, "%s::GetSingleton(),\n", metaTypeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	writeStringToFile("m_nestedTypeAliases = s_nestedTypeAliases;\n", file, indentation);
	writeStringToFile("m_nestedTypeAliasCount = paf_array_size_of(s_nestedTypeAliases);\n", file, indentation);
}

void writeMetaConstructor_Scope(MemberNode* memberNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	assert(0 != memberNode->m_enclosing);
	if (snt_namespace == memberNode->m_enclosing->m_nodeType)
	{
		writeStringToFile("::paf::NameSpace::GetGlobalNameSpace()", file, indentation);

		std::vector<ScopeNode*> enclosings;
		memberNode->getEnclosings(enclosings);
		assert(!enclosings.empty());
		size_t count = enclosings.size();
		for (size_t i = 1; i < count; ++i)//enclosings[0] is global namespace 
		{
			ScopeNode* enclosing = enclosings[i];
			if (snt_namespace == enclosing->m_nodeType)
			{
				sprintf_s(buf, "->getNameSpace(\"%s\")", enclosing->m_name->m_str.c_str());
				writeStringToFile(buf, file);
			}
			else
			{
				break;
			}
		}
		writeStringToFile("->registerMember(this);\n", file);
	}
	else if (snt_class == memberNode->m_enclosing->m_nodeType)
	{
		std::string metaTypeName;
		GetMetaTypeFullName(metaTypeName, memberNode->m_enclosing, templateArguments);
		sprintf_s(buf, "m_enclosing = %s::GetSingleton();\n", metaTypeName.c_str());
		writeStringToFile(buf, file, indentation);
	}
	else
	{
		assert(false);
	}
}

void writeMetaConstructor(ClassNode* classNode,
	TemplateArguments* templateArguments,
	std::vector<MemberNode*>& nestedTypeNodes,
	std::vector<MemberNode*>& nestedTypeAliasNodes,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	std::vector<MethodNode*>& staticMethodNodes,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	std::vector<MethodNode*>& methodNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	std::string localClassName;
	std::string className;
	std::string metaClassName;
	classNode->getLocalName(localClassName, templateArguments);
	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);


	std::sort(nestedTypeNodes.begin(), nestedTypeNodes.end(), CompareMemberNodeByName());
	std::sort(nestedTypeAliasNodes.begin(), nestedTypeAliasNodes.end(), CompareMemberNodeByName());
	//std::sort(fieldNodes.begin(), fieldNodes.end(), CompareMemberNodeByName());
	//std::sort(propertyNodes.begin(), propertyNodes.end(), CompareMemberNodeByName());
	std::sort(methodNodes.begin(), methodNodes.end(), CompareMethodNode());
	//std::sort(staticFieldNodes.begin(), staticFieldNodes.end(), CompareMemberNodeByName());
	//std::sort(staticPropertyNodes.begin(), staticPropertyNodes.end(), CompareMemberNodeByName());
	std::sort(staticMethodNodes.begin(), staticMethodNodes.end(), CompareMethodNode());

	sprintf_s(buf, "%s::%s() : ::paf::ClassType(\"%s\", ::paf::MetaCategory::%s, \"%s\")\n",
		metaClassName.c_str(), metaClassName.c_str(),
		localClassName.c_str(),
		classNode->m_category ? classNode->m_category->m_str.c_str() : "object",
		classNode->getSourceFilePath().c_str());

	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeMetaConstructor_Attributeses(attributesOffsets, classNode, staticFieldNodes, staticPropertyNodes, staticMethodNodes,
		fieldNodes, propertyNodes, methodNodes, file, indentation + 1);

	writeMetaConstructor_attributesForType(attributesOffsets, classNode, file, indentation + 1);

	sprintf_s(buf, "m_size = sizeof(%s);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);

	sprintf_s(buf, "m_is_introspectable = std::is_base_of_v<::paf::Introspectable, %s>;\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	sprintf_s(buf, "m_is_trivially_destructible = std::is_trivially_destructible_v<%s>;\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);

	writeMetaConstructor_BaseClasses(classNode, templateArguments, file, indentation + 1);
	writeMetaConstructor_ClassTypeIterators(classNode, templateArguments, file, indentation + 1);
	writeMetaConstructor_NestedTypes(classNode, templateArguments, nestedTypeNodes, file, indentation + 1);
	writeMetaConstructor_NestedTypeAliases(classNode, templateArguments, nestedTypeAliasNodes, file, indentation + 1);
	writeMetaConstructor_Fields(classNode, templateArguments, attributesOffsets, staticFieldNodes, true, file, indentation + 1);
	writeMetaConstructor_Properties(classNode, templateArguments, attributesOffsets, staticPropertyNodes, true, file, indentation + 1);
	writeMetaConstructor_Methods(classNode, templateArguments, attributesOffsets, staticMethodNodes, true, file, indentation + 1);

	writeMetaConstructor_Fields(classNode, templateArguments, attributesOffsets, fieldNodes, false, file, indentation + 1);
	writeMetaConstructor_Properties(classNode, templateArguments, attributesOffsets, propertyNodes, false, file, indentation + 1);
	writeMetaConstructor_Methods(classNode, templateArguments, attributesOffsets, methodNodes, false, file, indentation + 1);

	std::sort(fieldNodes.begin(), fieldNodes.end(), CompareMemberNodeByName());
	std::sort(propertyNodes.begin(), propertyNodes.end(), CompareMemberNodeByName());
	std::sort(staticFieldNodes.begin(), staticFieldNodes.end(), CompareMemberNodeByName());
	std::sort(staticPropertyNodes.begin(), staticPropertyNodes.end(), CompareMemberNodeByName());

	writeMetaConstructor_Member(nestedTypeNodes, nestedTypeAliasNodes, staticFieldNodes, staticPropertyNodes,
		staticMethodNodes, fieldNodes, propertyNodes, methodNodes, file, indentation + 1);

	writeMetaConstructor_Scope(classNode, templateArguments, file, indentation + 1);

	writeStringToFile("}\n\n", file, indentation);
}

void writeEnumMetaConstructor_Enumerators(EnumNode* enumNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t> attributesOffsets,
	std::vector<EnumeratorNode*>& enumerators, FILE* file, int indentation)
{
	char buf[4096];
	char strAttributes[256];
	if (enumerators.empty())
	{
		return;
	}
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, enumNode, templateArguments);
	size_t count = enumerators.size();

	writeStringToFile("static ::paf::Enumerator s_enumerators[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < count; ++i)
	{
		EnumeratorNode* enumerator = enumerators[i];
		if (enumerator->m_attributeList)
		{
			auto it = attributesOffsets.find(enumerator);
			assert(it != attributesOffsets.end());
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		std::string enumScopeName;
		enumNode->m_enclosing->getNativeName(enumScopeName, 0);
		if (enumNode->isStronglyTypedEnum())
		{
			sprintf_s(buf, "::paf::Enumerator(\"%s\", %s, this, int(%s::%s::%s)),\n",
				enumerator->m_name->m_str.c_str(), strAttributes,
				/*metaClassName.c_str(),*/ enumScopeName.c_str(), enumNode->m_name->m_str.c_str(), enumerator->m_name->m_str.c_str());
		}
		else
		{
			//%s::GetSingleton()
			sprintf_s(buf, "::paf::Enumerator(\"%s\", %s, this, %s::%s),\n",
				enumerator->m_name->m_str.c_str(), strAttributes,
				/*metaClassName.c_str(),*/ enumScopeName.c_str(), enumerator->m_name->m_str.c_str());
		}
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	writeStringToFile("m_enumerators = s_enumerators;\n", file, indentation);
	writeStringToFile("m_enumeratorCount = paf_array_size_of(s_enumerators);\n", file, indentation);
}

void writeEnumMetaConstructor(EnumNode* enumNode, TemplateArguments* templateArguments, std::vector<EnumeratorNode*>& enumerators, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string metaTypeName;
	enumNode->getNativeName(typeName, 0);
	GetMetaTypeFullName(metaTypeName, enumNode, templateArguments);

	sprintf_s(buf, "%s::%s() : ::paf::EnumType(\"%s\", \"%s\")\n",
		metaTypeName.c_str(),
		metaTypeName.c_str(),
		enumNode->m_name->m_str.c_str(),
		enumNode->getSourceFilePath().c_str());

	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeEnumMetaConstructor_Attributeses(attributesOffsets, enumNode, enumerators, file, indentation + 1);
	writeMetaConstructor_attributesForType(attributesOffsets, enumNode, file, indentation + 1);

	sprintf_s(buf, "static_assert(sizeof(%s) <= sizeof(int), \"enum size is too large\");\n", typeName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	
	sprintf_s(buf, "m_size = sizeof(%s);\n", typeName.c_str());
	writeStringToFile(buf, file, indentation + 1);

	writeEnumMetaConstructor_Enumerators(enumNode, templateArguments, attributesOffsets, enumerators, file, indentation + 1);
	writeMetaConstructor_Scope(enumNode, templateArguments, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaGetSingletonImpls(MemberNode* memberNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, memberNode, templateArguments);

	sprintf_s(buf, "%s* %s::GetSingleton()\n", metaClassName.c_str(), metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "static %s* s_instance = 0;\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	sprintf_s(buf, "static char s_buffer[sizeof(%s)];\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("if(0 == s_instance)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "s_instance = (%s*)s_buffer;\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 2);
	sprintf_s(buf, "new (s_buffer)%s;\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);
	writeStringToFile("return s_instance;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void MetaSourceFileGenerator::generateCode_Typedef(FILE* file, TypedefNode* typedefNode, TemplateArguments* templateArguments, int indentation)
{
	if (typedefNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string typeName;
	std::string metaTypeName;
	typedefNode->getNativeName(typeName, templateArguments);
	GetMetaTypeFullName(metaTypeName, typedefNode, templateArguments);

	sprintf_s(buf, "%s::%s() : TypeAlias(\"%s\", RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), \"%s\")\n",
		metaTypeName.c_str(), metaTypeName.c_str(), typedefNode->m_name->m_str.c_str(), typeName.c_str(), typedefNode->getSourceFilePath().c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeTypeDefMetaConstructor_Attributeses(attributesOffsets, typedefNode, file, indentation + 1);
	writeMetaConstructor_attributesForType(attributesOffsets, typedefNode, file, indentation + 1);

	writeMetaConstructor_Scope(typedefNode, templateArguments, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
	writeMetaGetSingletonImpls(typedefNode, templateArguments, file, indentation);
}

void MetaSourceFileGenerator::generateCode_TypeDeclaration(FILE* file, TypeDeclarationNode* typeDeclarationNode, TemplateArguments* templateArguments, int indentation)
{
	if (typeDeclarationNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string typeName;
	std::string metaTypeName;
	typeDeclarationNode->getNativeName(typeName, templateArguments);
	GetMetaTypeFullName(metaTypeName, typeDeclarationNode, templateArguments);

	sprintf_s(buf, "%s::%s() : TypeAlias(\"%s\", RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), \"%s\")\n",
		metaTypeName.c_str(), metaTypeName.c_str(), typeDeclarationNode->m_name->m_str.c_str(), typeName.c_str(), typeDeclarationNode->getSourceFilePath().c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeTypeDefMetaConstructor_Attributeses(attributesOffsets, typeDeclarationNode, file, indentation + 1);
	writeMetaConstructor_attributesForType(attributesOffsets, typeDeclarationNode, file, indentation + 1);
	writeMetaConstructor_Scope(typeDeclarationNode, templateArguments, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
	writeMetaGetSingletonImpls(typeDeclarationNode, templateArguments, file, indentation);
}

void writeOverrideMethodParameter(MethodNode* methodNode, ParameterNode* parameterNode, FILE* file);
void writeOverrideMethodOutputParameter(MethodNode* methodNode, VariableTypeNode* resultNode, FILE* file);
std::string CalcCompoundTypeName(TypeNameNode* typeNameNode, TypeCompound typeCompound, ParameterPassing passing, ScopeNode* scopeNode);


void writeInterfaceMethodImpl_AssignThis(ClassNode* classNode, MethodNode* methodNode, FILE* file, int indentation)
{
	std::string className;
	classNode->getFullName(className, 0);
	char buf[4096];
	sprintf_s(buf, "__self__.assignRawPtr<%s>(this);\n", className.c_str());
	writeStringToFile(buf, file, indentation);
}

void writeInterfaceMethodImpl_AssignParam(ParameterNode* parameterNode, size_t argIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, 0);

	switch (parameterNode->m_typeCompound)
	{
	case tc_raw_ptr:
		sprintf_s(buf, "__arguments__[%zd].assignRawPtr<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_raw_array:
		sprintf_s(buf, "__arguments__[%zd].assignRawArray<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_shared_ptr:
		sprintf_s(buf, "__arguments__[%zd].assignSharedPtr<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_shared_array:
		sprintf_s(buf, "__arguments__[%zd].assignSharedArray<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	default:
		sprintf_s(buf, "__arguments__[%zd].assignRawPtr<%s>(&%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
	}
	writeStringToFile(buf, file, indentation);
}

void writeInterfaceMethodImpl_CastOutputParam(VariableTypeNode* resultNode, size_t argIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, resultNode->m_typeName, 0);
	switch (resultNode->m_typeCompound)
	{
	case tc_none:
		sprintf_s(buf, "__results__[%zd].castToValue<%s>(output%zd);\n", argIndex, typeName.c_str(), argIndex);
		break;
	case tc_raw_ptr:
		sprintf_s(buf, "__results__[%zd].castToRawPtr<%s>(output%zd);\n", argIndex, typeName.c_str(), argIndex);
		break;
	case tc_raw_array:
		sprintf_s(buf, "__results__[%zd].castToRawArray<%s>(output%zd);\n", argIndex, typeName.c_str(), argIndex);
		break;
	case tc_shared_ptr:
		sprintf_s(buf, "__results__[%zd].castToSharedPtr<%s>(output%zd);\n", argIndex, typeName.c_str(), argIndex);
		break;
	case tc_shared_array:
		sprintf_s(buf, "__results__[%zd].castToSharedArray<%s>(output%zd);\n", argIndex, typeName.c_str(), argIndex);
		break;
	}
	writeStringToFile(buf, file, indentation);
}

void writeInterfaceMethodImpl_CastResult(MethodNode* methodNode, VariableTypeNode* resultNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, resultNode->m_typeName, 0);
	std::string strResultType = CalcCompoundTypeName(resultNode->m_typeName, resultNode->m_typeCompound, pp_value, methodNode->getProgramNode());

	sprintf_s(buf, "%s __res__;\n", strResultType.c_str());
	writeStringToFile(buf, file, indentation);
	switch (resultNode->m_typeCompound)
	{
	case tc_none:
		sprintf_s(buf, "__results__[0].castToValue<%s>(__res__);\n", typeName.c_str());
		break;
	case tc_raw_ptr:
		sprintf_s(buf, "__results__[0].castToRawPtr<%s>(__res__);\n", typeName.c_str());
		break;
	case tc_raw_array:
		sprintf_s(buf, "__results__[0].castToRawArray<%s>(__res__);\n", typeName.c_str());
		break;
	case tc_shared_ptr:
		sprintf_s(buf, "__results__[0].castToSharedPtr<%s>(__res__);\n", typeName.c_str());
		break;
	case tc_shared_array:
		sprintf_s(buf, "__results__[0].castToSharedArray<%s>(__res__);\n", typeName.c_str());
		break;
	}
	writeStringToFile(buf, file, indentation);
	writeStringToFile("return __res__;\n", file, indentation);
}

void writeInterfaceMethodImpl(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string subclassProxyName;
	GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);
	IdentifyNode* methodNameNode = methodNode->m_nativeName ? methodNode->m_nativeName : methodNode->m_name;

	VariableTypeNode* resultNode = nullptr;
	size_t startOutputParam = 0;
	std::vector<VariableTypeNode*> resultNodes;
	if (methodNode->m_resultList)
	{
		methodNode->m_resultList->collectVariableTypeNodes(resultNodes);
		if (!methodNode->m_voidResult)
		{
			resultNode = resultNodes.front();
			startOutputParam = 1;
		}
	}
	size_t resultCount = resultNodes.size();

	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t paramCount = parameterNodes.size();

	std::string resultName;
	if (resultNode)
	{
		resultName = CalcCompoundTypeName(resultNode->m_typeName, resultNode->m_typeCompound, pp_value, methodNode->getProgramNode());
	}
	else if(methodNode->m_voidResult)
	{
		resultName = "void";
	}
	sprintf_s(buf, "%s %s::%s( ", resultName.c_str(), subclassProxyName.c_str(), methodNameNode->m_str.c_str());
	writeStringToFile(buf, file, indentation);

	for (size_t i = startOutputParam; i < resultCount; ++i)
	{
		writeOverrideMethodOutputParameter(methodNode, resultNodes[i], file);
		sprintf_s(buf, "output%zd", i);
		writeStringToFile(buf, file);
		if (i + 1 < resultCount || 0 != paramCount)
		{
			writeStringToFile(", ", file);
		}
	}

	for (size_t i = 0; i < paramCount; ++i)
	{
		if (0 != i)
		{
			writeStringToFile(", ", file);
		}
		writeOverrideMethodParameter(methodNode, parameterNodes[i], file);
	}
	writeStringToFile(")", file);
	if (methodNode->m_constant)
	{
		writeStringToFile(" const", file, indentation);
	}
	writeStringToFile("\n", file);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("::paf::Variant __self__;\n", file, indentation + 1);
	if (0 < resultCount)
	{
		sprintf_s(buf, "::paf::Variant __results__[%zd];\n", resultCount);
		writeStringToFile(buf, file, indentation + 1);
	}
	if (0 < paramCount)
	{
		sprintf_s(buf, "::paf::Variant __arguments__[%zd];\n", paramCount);
		writeStringToFile(buf, file, indentation + 1);
	}


	writeInterfaceMethodImpl_AssignThis(classNode, methodNode, file, indentation + 1);
	for (size_t i = 0; i < paramCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		writeInterfaceMethodImpl_AssignParam(parameterNode, i, file, indentation + 1);
	}
	writeStringToFile("if(m_subclassInvoker)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "::paf::ErrorCode __error__ = m_subclassInvoker->invoke(\"%s\", &__self__, %s, %zd, %s, %zd);\n",
		methodNode->m_name->m_str.c_str(), resultCount > 0 ? "__results__" : "nullptr", resultCount, paramCount > 0 ? "__arguments__" : "nullptr", paramCount);
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);

	for (size_t i = startOutputParam; i < resultCount; ++i)
	{
		writeInterfaceMethodImpl_CastOutputParam(resultNodes[i], i, file, indentation + 1);
	}
	if (resultNode)
	{
		writeInterfaceMethodImpl_CastResult(methodNode, resultNode, file, indentation + 1);
	}
	writeStringToFile("}\n\n", file, indentation);
}

void writeInterfaceMethodsImpl(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	std::vector<MethodNode*> methodNodes;
	classNode->collectOverrideMethods(methodNodes, templateArguments);
	size_t count = methodNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		assert(snt_method == methodNode->m_nodeType && methodNode->m_override && methodNode->isVirtual());
		writeInterfaceMethodImpl(classNode, templateArguments, methodNode, file, indentation);
	}
}


void MetaSourceFileGenerator::generateCode_SubclassProxy(FILE* file, ClassNode* classNode, TemplateArguments* templateArguments, int indentation)
{
	char buf[4096];
	std::string subclassProxyName;
	GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);

	sprintf_s(buf, "%s::%s(::paf::SubclassInvoker* subclassInvoker)\n", subclassProxyName.c_str(), subclassProxyName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("m_subclassInvoker = subclassInvoker;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	sprintf_s(buf, "%s::~%s()\n", subclassProxyName.c_str(), subclassProxyName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("delete m_subclassInvoker;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	writeInterfaceMethodsImpl(classNode, templateArguments, file, indentation);
}

