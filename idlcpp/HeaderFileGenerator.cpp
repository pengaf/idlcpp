#include "HeaderFileGenerator.h"
#include "Utility.h"
#include "SourceFile.h"
#include "ProgramNode.h"
#include "NamespaceNode.h"
#include "TokenNode.h"
#include "IdentifyNode.h"
#include "EnumeratorListNode.h"
#include "ScopeNameListNode.h"
#include "ScopeNameNode.h"
#include "MemberListNode.h"
#include "EnumeratorNode.h"
#include "EnumNode.h"
#include "ClassNode.h"
#include "DelegateNode.h"
#include "TemplateParametersNode.h"
#include "TemplateParameterListNode.h"
#include "TypeNameListNode.h"
#include "TypeNameNode.h"
#include "FieldNode.h"
#include "PropertyNode.h"
#include "MethodNode.h"
#include "ParameterNode.h"
#include "ParameterListNode.h"
#include "TypedefNode.h"
#include "Compiler.h"
#include "Options.h"
#include "Platform.h"
#include <assert.h>

void generateCode_Token(FILE* file, TokenNode* tokenNode, int indentation)
{
	g_compiler.outputEmbededCodes(file, tokenNode);

	if(indentation > 0)
	{
		writeStringToFile("", 0, file, indentation);
	}
	if(tokenNode->m_nodeType < 256)
	{
		char ch = tokenNode->m_nodeType;
		//if ('+' == ch || '-' == ch)
		//{
		//	ch = '*';
		//}
		writeStringToFile(&ch, 1, file);
	}
	else
	{
		assert(snt_begin_output < tokenNode->m_nodeType && tokenNode->m_nodeType < snt_end_output);
		const char* str = g_keywordTokens[tokenNode->m_nodeType - snt_begin_output - 1];
		if (isNumAlpha_(GetLastWrittenChar()))
		{
			writeSpaceToFile(file);
		}
		writeStringToFile(str, file);
	}
}

void generateCode_TokenForOperator(FILE* file, TokenNode* tokenNode, int indentation)
{
	g_compiler.outputEmbededCodes(file, tokenNode);

	if (indentation > 0)
	{
		writeStringToFile("", 0, file, indentation);
	}
	if (tokenNode->m_nodeType < 256)
	{
		char ch = tokenNode->m_nodeType;
		writeStringToFile(&ch, 1, file);
	}
	else
	{
		assert(snt_begin_output < tokenNode->m_nodeType && tokenNode->m_nodeType < snt_end_output);
		const char* str = g_keywordTokens[tokenNode->m_nodeType - snt_begin_output - 1];
		if (isNumAlpha_(GetLastWrittenChar()))
		{
			writeSpaceToFile(file);
		}
		writeStringToFile(str, file);
	}
}

void generateCode_Identify(FILE* file, IdentifyNode* identifyNode, int indentation, bool addSpace = true)
{
	g_compiler.outputEmbededCodes(file, identifyNode);
	if(indentation > 0)
	{
		writeStringToFile("", 0, file, indentation);
	}
	if(addSpace && isNumAlpha_(GetLastWrittenChar()))
	{
		writeSpaceToFile(file);
	}
	writeStringToFile(identifyNode->m_str.c_str(), identifyNode->m_str.length(), file);
};

void generateCode_TypeName__(FILE* file, TypeNameNode* typeNameNode, ScopeNode* scopeNode, bool addKeyword, bool addSpace)
{
	if(typeNameNode->m_keyword)
	{
		typeNameNode->m_keyword->outputEmbededCodes(file, addSpace);
	}
	if(typeNameNode->m_scopeNameList)
	{
		typeNameNode->m_scopeNameList->m_scopeName->m_name->outputEmbededCodes(file, addSpace);
	}
	if (addKeyword && typeNameNode->underTemplateParameter())
	{
		writeStringToFile("typename ", file);
	}
	std::string typeName;
	typeNameNode->getRelativeName(typeName, scopeNode);
	writeStringToFile(typeName.c_str(), file);
}

void generateCode_CompoundTypeName(FILE* file, TypeNameNode* typeNameNode, TypeCompound typeCompound, ScopeNode* scopeNode, bool addKeyword, int indentation)
{
	writeIndentationsToFile(file, indentation);
	switch (typeCompound)
	{
	case tc_raw_ptr:
		writeStringToFile("::paf::RawPtr<", file);
		break;
	case tc_raw_array:
		writeStringToFile("::paf::RawArray<", file);
		break;
	case tc_borrowed_ptr:
		writeStringToFile("::paf::BorrowedPtr<", file);
		break;
	case tc_borrowed_array:
		writeStringToFile("::paf::BorrowedArray<", file);
		break;
	case tc_unique_ptr:
		writeStringToFile("::paf::UniquePtr<", file);
		break;
	case tc_unique_array:
		writeStringToFile("::paf::UniqueArray<", file);
		break;
	case tc_shared_ptr:
		writeStringToFile("::paf::SharedPtr<", file);
		break;
	case tc_shared_array:
		writeStringToFile("::paf::SharedArray<", file);
		break;
	}
	generateCode_TypeName__(file, typeNameNode, scopeNode, true, 0 == indentation);
	switch (typeCompound)
	{
	case tc_raw_ptr:
	case tc_raw_array:
	case tc_borrowed_ptr:
	case tc_borrowed_array:
	case tc_unique_ptr:
	case tc_unique_array:
	case tc_shared_ptr:
	case tc_shared_array:
		writeStringToFile(">", file);
		break;
	}
}

void generateCode_ParameterPassing(FILE* file, ParameterPassing passing)
{
	switch (passing)
	{
	case pp_value:
		writeStringToFile(" ", file);
		break;
	case pp_reference:
		writeStringToFile("& ", file);
		break;
	case pp_const_reference:
		writeStringToFile(" const & ", file);
		break;
	case pp_rvalue_reference:
		writeStringToFile(" && ", file);
		break;
	case pp_const_rvalue_reference:
		writeStringToFile(" const && ", file);
		break;
	}
}

void generateCode_Parameter(FILE* file, ParameterNode* parameterNode, ScopeNode* scopeNode)
{
	generateCode_CompoundTypeName(file, parameterNode->m_typeName, parameterNode->m_typeCompound, scopeNode, true, 0);
	generateCode_ParameterPassing(file, parameterNode->m_passing);
	generateCode_Identify(file, parameterNode->m_name, 0);
	if (parameterNode->m_defaultDenote)
	{
		generateCode_Token(file, parameterNode->m_defaultDenote, 0);
	}
};

void generateCode_ParameterList(FILE* file, ParameterListNode* parameterListNode, ScopeNode* scopeNode)
{
	std::vector<std::pair<TokenNode*, ParameterNode*>> parameterNodes;
	parameterListNode->collectParameterNodes(parameterNodes);
	size_t parameterCount = parameterNodes.size();
	for (size_t i = 0; i < parameterCount; ++i)
	{
		if (parameterNodes[i].first)
		{
			generateCode_Token(file, parameterNodes[i].first, 0);
		}
		generateCode_Parameter(file, parameterNodes[i].second, scopeNode);
	}
}



void HeaderFileGenerator::generateCode(FILE* dstFile, SourceFile* sourceFile, const char* fileName)
{
	generateCode_Program(dstFile, sourceFile, fileName);
}

void HeaderFileGenerator::generateCode_Program(FILE* file, SourceFile* sourceFile, const char* fileName)
{
	std::string pafcorePath;
	GetRelativePath(pafcorePath, fileName, g_options.m_pafcorePath.c_str());
	FormatPathForInclude(pafcorePath);

	writeStringToFile("#pragma once\n\n", file);

	g_compiler.outputUsedTypes(file, sourceFile, pafcorePath.c_str());
	writeStringToFile("namespace paf{ class ClassType; }\n", file);
	if (sourceFile->m_hasCollectionProperty)
	{
		writeStringToFile("namespace paf{ class Iterator; }\n", file);
	}

	generateCode_Namespace(file, sourceFile->m_syntaxTree, -1);

	g_compiler.outputEmbededCodes(file, 0);
}

void HeaderFileGenerator::generateCode_Namespace(FILE* file, NamespaceNode* namespaceNode, int indentation)
{
	if (namespaceNode->isNoCode())
	{
		g_compiler.outputEmbededCodes(file, namespaceNode->m_filterNode);
		file = 0;
	}

	if(!namespaceNode->isGlobalNamespace())
	{
		generateCode_Token(file, namespaceNode->m_keyword, indentation);
		generateCode_Identify(file, namespaceNode->m_name, 0);
		generateCode_Token(file, namespaceNode->m_leftBrace, indentation);
	}
	std::vector<MemberNode*> memberNodes;
	namespaceNode->m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(memberNode), indentation + 1);
			break;
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(memberNode), indentation + 1);
			break;
		case snt_delegate:
			generateCode_Delegate(file, static_cast<DelegateNode*>(memberNode), indentation + 1);
			break;
		case snt_template_class_instance:
			break;
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(memberNode), indentation + 1);
			break;
		case snt_namespace:
			generateCode_Namespace(file, static_cast<NamespaceNode*>(memberNode), indentation + 1);
			break;
		case snt_type_declaration:
			break;
		default:
			assert(false);
		}
	}

	if(!namespaceNode->isGlobalNamespace())
	{
		generateCode_Token(file, namespaceNode->m_rightBrace, indentation);
	}
}


void HeaderFileGenerator::generateCode_Typedef(FILE* file, TypedefNode* typedefNode, int indentation)
{
	if (typedefNode->isNoCode())
	{
		g_compiler.outputEmbededCodes(file, typedefNode->m_filterNode);
		file = 0;
	}
	if(0 != typedefNode->m_typeName)
	{
		generateCode_Token(file, typedefNode->m_keyword, indentation);	
		generateCode_TypeName__(file, typedefNode->m_typeName, typedefNode->m_enclosing, true, 0);
		writeSpaceToFile(file);
		generateCode_Identify(file, typedefNode->m_name, 0);
		writeStringToFile(";", 1, file);
	}
}

void HeaderFileGenerator::generateCode_Enum(FILE* file, EnumNode* enumNode, int indentation)
{
	if (enumNode->isNoCode())
	{
		g_compiler.outputEmbededCodes(file, enumNode->m_filterNode);
		file = 0;
	}
	else if (enumNode->m_nativeName)
	{
		g_compiler.outputEmbededCodes(file, enumNode->m_keyword);
		file = 0;
	}

	generateCode_Token(file, enumNode->m_keyword, indentation);
	if (enumNode->m_keyword2)
	{
		generateCode_Token(file, enumNode->m_keyword2, 0);
	}
	generateCode_Identify(file, enumNode->m_name, 0);
	generateCode_Token(file, enumNode->m_leftBrace, indentation);

	std::vector<std::pair<TokenNode*, EnumeratorNode*>> enumeratorNodes;
	enumNode->m_enumeratorList->collectEnumeratorNodes(enumeratorNodes);
	size_t itemCount = enumeratorNodes.size();
	for(size_t i = 0; i < itemCount; ++i)
	{
		if(0 != enumeratorNodes[i].first)
		{
			generateCode_Token(file, enumeratorNodes[i].first, 0);
		}
		generateCode_Identify(file, enumeratorNodes[i].second->m_name, indentation + 1);
	}
	generateCode_Token(file, enumNode->m_rightBrace, indentation);
	generateCode_Token(file, enumNode->m_semicolon, 0);
}

void HeaderFileGenerator::generateCode_Class(FILE* file, ClassNode* classNode, int indentation)
{
	if (classNode->isNoCode())
	{
		g_compiler.outputEmbededCodes(file, classNode->m_filterNode);
		file = 0;
	}
	else if (classNode->m_nativeName)
	{
		g_compiler.outputEmbededCodes(file, classNode->m_keyword);
		file = 0;
	}

	if(classNode->m_templateParametersNode)
	{
		generateCode_Token(file, classNode->m_templateParametersNode->m_keyword, indentation);
		generateCode_Token(file, classNode->m_templateParametersNode->m_leftBracket, 0);
		std::vector<std::pair<TokenNode*, IdentifyNode*>> parameterNodes;
		classNode->m_templateParametersNode->collectParameterNodes(parameterNodes);

		size_t count = parameterNodes.size();
		for(size_t i = 0; i < count; ++i)
		{
			if(parameterNodes[i].first)
			{
				generateCode_Token(file, parameterNodes[i].first, 0);
			}
			writeStringToFile("typename ", file);
			generateCode_Identify(file, parameterNodes[i].second, 0);
		}
		generateCode_Token(file, classNode->m_templateParametersNode->m_rightBracket, 0);
	}

	generateCode_Token(file, classNode->m_keyword, indentation);
	generateCode_Identify(file, classNode->m_name, 0);


	if(classNode->m_baseList)
	{
		std::vector<std::pair<TokenNode*, TypeNameNode*>> typeNameNodes;
		classNode->m_baseList->collectTypeNameNodesNotNoCode(typeNameNodes);
		size_t baseCount = typeNameNodes.size();
		if (baseCount)
		{
			assert(0 != classNode->m_colon);
			writeSpaceToFile(file);
			generateCode_Token(file, classNode->m_colon, 0);
			writeSpaceToFile(file);
			for(size_t i = 0; i < baseCount; ++i)
			{
				if(typeNameNodes[i].first && 0 != i)
				{
					generateCode_Token(file, typeNameNodes[i].first, 0);
				}
				writeStringToFile("public ", file);
				generateCode_TypeName__(file, typeNameNodes[i].second, classNode->m_enclosing, false, 0);
			}
		}
	}

	generateCode_Token(file, classNode->m_leftBrace, indentation);
	writeStringToFile("\n", file);
	writeStringToFile("public:\n", file, indentation);

	std::vector<MemberNode*> memberNodes;
	classNode->m_memberList->collectMemberNodes(memberNodes);

	auto it = classNode->m_additionalMethods.begin();
	auto end = classNode->m_additionalMethods.end();
	for (; it != end; ++it)
	{
		MethodNode* methodNode = *it;
		if (!methodNode->isNoCode())
		{
			memberNodes.push_back(methodNode);
		}
	}

	size_t memberCount = memberNodes.size();
	for (size_t i = 0; i < memberCount; ++i)
	{
		char buf[4096];
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_class:
		{
			ClassNode* nestedClassNode = static_cast<ClassNode*>(memberNode);
			if (0 == nestedClassNode->m_nativeName)
			{
				sprintf_s(buf, "%s%s;\n", g_keywordTokens[nestedClassNode->m_keyword->m_nodeType - snt_begin_output - 1],
					nestedClassNode->m_name->m_str.c_str());
				writeStringToFile(buf, file, indentation + 1);
			}
		}
		break;
		case snt_enum:
		{
			EnumNode* nestedEnumNode = static_cast<EnumNode*>(memberNode);
			if (0 == nestedEnumNode->m_nativeName)
			{
				sprintf_s(buf, "%s%s%s;\n", g_keywordTokens[nestedEnumNode->m_keyword->m_nodeType - snt_begin_output - 1],
					nestedEnumNode->m_keyword2 ? "class " : "",
					nestedEnumNode->m_name->m_str.c_str());
				writeStringToFile(buf, file, indentation + 1);
			}
		}
		break;
		}
	}
	if (!classNode->isNoMeta())
	{
		writeStringToFile("static ::paf::ClassType* GetType();\n", file, indentation + 1);
		writeStringToFile("::paf::ClassType* getType()\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return GetType();\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
		writeStringToFile("void* getAddress()\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return this;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}
	for(size_t i = 0; i < memberCount; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch(memberNode->m_nodeType)
		{
		case snt_field:
			generateCode_Field(file, static_cast<FieldNode*>(memberNode), indentation + 1);
			break;
		case snt_property:
			generateCode_Property(file, static_cast<PropertyNode*>(memberNode), indentation + 1);
			break;
		case snt_method:
			generateCode_Method(file, static_cast<MethodNode*>(memberNode), indentation + 1);
			break;
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(memberNode), indentation + 1);
			break;
		case snt_delegate:
			generateCode_Delegate(file, static_cast<DelegateNode*>(memberNode), indentation + 1);
			break;
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(memberNode), indentation + 1);
			break;
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(memberNode), indentation + 1);
			break;
		case snt_type_declaration:
			break;
		default:
			assert(false);
		}
	}

	generateCode_Token(file, classNode->m_rightBrace, indentation);
	generateCode_Token(file, classNode->m_semicolon, 0);
}

void HeaderFileGenerator::generateCode_Delegate(FILE* file, DelegateNode* delegateNode, int indentation)
{
	if (delegateNode->isNoCode())
	{
		g_compiler.outputEmbededCodes(file, delegateNode->m_filterNode);
		file = 0;
	}
	generateCode_Token(file, delegateNode->m_keyword, indentation);
	generateCode_Identify(file, delegateNode->m_name, 0);
	
	writeStringToFile(" : public ::paf::Delegate\n", file);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("public:\n", file, indentation);
	
	int methodIndentation = indentation + 1;

	generateCode_Identify(file, delegateNode->m_name, methodIndentation);
	writeStringToFile("() = default;\n", file, 0);
	generateCode_Identify(file, delegateNode->m_name, methodIndentation);
	writeStringToFile("(const ", file, 0);
	generateCode_Identify(file, delegateNode->m_name, 0);
	writeStringToFile("&) = delete;\n", file, 0);

	if (0 != delegateNode->m_resultTypeName)
	{
		generateCode_CompoundTypeName(file, delegateNode->m_resultTypeName, delegateNode->m_resultTypeCompound, delegateNode->m_enclosing, true, methodIndentation);
		methodIndentation = 0;
		writeSpaceToFile(file);
	}
	writeStringToFile("invoke", file, methodIndentation);
	generateCode_Token(file, delegateNode->m_leftParenthesis, 0);
	generateCode_ParameterList(file, delegateNode->m_parameterList, delegateNode->m_enclosing);
	generateCode_Token(file, delegateNode->m_rightParenthesis, 0);
	generateCode_Token(file, delegateNode->m_semicolon, 0);
	writeStringToFile("\n", file);

	writeStringToFile("typedef ", file, indentation + 1);
	if (0 != delegateNode->m_resultTypeName)
	{
		generateCode_CompoundTypeName(file, delegateNode->m_resultTypeName, delegateNode->m_resultTypeCompound, delegateNode->m_enclosing, true, 0);
		writeSpaceToFile(file);
	}
	writeStringToFile("(*CallBackFunction)(void* userData, ", file, 0);
	generateCode_ParameterList(file, delegateNode->m_parameterList, delegateNode->m_enclosing);
	writeStringToFile(");\n", file);
	writeStringToFile("::paf::FunctionCallBack* addFunction(CallBackFunction function, void* userData)\n", file, indentation + 1);
	writeStringToFile("{return Delegate::addFunction(function, userData);}\n", file, indentation + 1);
	writeStringToFile("};\n", file, indentation);
}

void HeaderFileGenerator::generateCode_Field(FILE* file, FieldNode* fieldNode, int indentation)
{
	if (fieldNode->isNoCode())
	{
		g_compiler.outputEmbededCodes(file, fieldNode->m_filterNode);
		file = 0;
	}
	else if (fieldNode->m_nativeName)
	{
		TokenNode* firstToken;
		if (0 != fieldNode->m_static)
		{
			firstToken = fieldNode->m_static;
		}
		else
		{
			if (fieldNode->m_typeName->m_keyword)
			{
				firstToken = fieldNode->m_typeName->m_keyword;
			}
			else
			{
				firstToken = fieldNode->m_typeName->m_scopeNameList->m_scopeName->m_name;
			}
		}
		g_compiler.outputEmbededCodes(file, firstToken);
		file = 0;
	}
	ClassNode* classNode = static_cast<ClassNode*>(fieldNode->m_enclosing);
	if(fieldNode->m_static)
	{
		generateCode_Token(file, fieldNode->m_static, indentation);
		indentation = 0;
	}
	generateCode_CompoundTypeName(file, fieldNode->m_typeName, fieldNode->m_typeCompound, fieldNode->m_enclosing, true, indentation);
	writeSpaceToFile(file);
	generateCode_Identify(file, fieldNode->m_name, 0);
	if(fieldNode->m_leftBracket)
	{
		generateCode_Token(file, fieldNode->m_leftBracket, 0);
	}
	if(fieldNode->m_rightBracket)
	{
		generateCode_Token(file, fieldNode->m_rightBracket, 0);
	}
	generateCode_Token(file, fieldNode->m_semicolon, 0);
}

void HeaderFileGenerator::generateCode_Property_Get(FILE* file, PropertyNode* propertyNode, int indentation)
{
	if (propertyNode->m_get->m_nativeName)
	{
		TokenNode* firstToken;
		if (0 != propertyNode->m_modifier)
		{
			firstToken = propertyNode->m_modifier;
		}
		else
		{
			if (propertyNode->m_typeName->m_keyword)
			{
				firstToken = propertyNode->m_typeName->m_keyword;
			}
			else
			{
				firstToken = propertyNode->m_typeName->m_scopeNameList->m_scopeName->m_name;
			}
		}
		g_compiler.outputEmbededCodes(file, firstToken);
		file = 0;
	}

	ClassNode* classNode = static_cast<ClassNode*>(propertyNode->m_enclosing);
	if(propertyNode->m_modifier)
	{
		generateCode_Token(file, propertyNode->m_modifier, indentation);
		indentation = 0;
	}
	generateCode_CompoundTypeName(file, propertyNode->m_typeName, propertyNode->m_get->m_typeCompound, propertyNode->m_enclosing, true, indentation);

	writeSpaceToFile(file);
	generateCode_Token(file, propertyNode->m_get->m_keyword, 0);
	generateCode_Identify(file, propertyNode->m_name, 0, false);

	writeStringToFile("(", file);

	if (propertyNode->isArray())
	{
		writeStringToFile("size_t", file);
	}
	else if (propertyNode->isCollection())
	{
		writeStringToFile("::paf::Iterator*", file);
	}
	if (propertyNode->isStatic())
	{
		writeStringToFile(");", file);
	}
	else
	{
		writeStringToFile(") const;", file);
	}
}

void HeaderFileGenerator::generateCode_Property_Set(FILE* file, PropertyNode* propertyNode, int indentation)
{
	if (propertyNode->m_set->m_nativeName)
	{
		TokenNode* firstToken;
		if (0 != propertyNode->m_modifier)
		{
			firstToken = propertyNode->m_modifier;
		}
		else
		{
			if (propertyNode->m_typeName->m_keyword)
			{
				firstToken = propertyNode->m_typeName->m_keyword;
			}
			else
			{
				firstToken = propertyNode->m_typeName->m_scopeNameList->m_scopeName->m_name;
			}
		}
		g_compiler.outputEmbededCodes(file, firstToken);
		file = 0;
	}

	ClassNode* classNode = static_cast<ClassNode*>(propertyNode->m_enclosing);

	if(propertyNode->m_modifier)
	{
		generateCode_Token(file, propertyNode->m_modifier, indentation);
		indentation = 0;
	}
	generateCode_Token(file, propertyNode->m_set->m_keyword, indentation);
	generateCode_Identify(file, propertyNode->m_name, 0, false);
	
	writeStringToFile("(", file);
	
	if (propertyNode->isArray())
	{
		writeStringToFile("size_t index, ", file);
	}
	else if (propertyNode->isCollection())
	{
		writeStringToFile("::paf::Iterator* dstIterator, size_t dstCount, ", file);
	}
	generateCode_CompoundTypeName(file, propertyNode->m_typeName, propertyNode->m_set->m_typeCompound, propertyNode->m_enclosing, true, 0);
	generateCode_ParameterPassing(file, propertyNode->m_set->m_passing);
	if (propertyNode->isCollection())
	{
		writeStringToFile(", size_t count", file);
	}
	writeStringToFile(");", file);	
}

void HeaderFileGenerator::generateCode_Property_Size(FILE* file, PropertyNode* propertyNode, int indentation)
{
	if (propertyNode->isStatic())
	{
		writeStringToFile("static ", file, indentation);
		indentation = 0;
	}
	writeStringToFile("size_t size_", file, indentation);
	writeStringToFile(propertyNode->m_name->m_str.c_str(), file);
	if (propertyNode->isStatic())
	{
		writeStringToFile("();", file);
	}
	else
	{
		writeStringToFile("() const;", file);
	}
}

void HeaderFileGenerator::generateCode_Property_Iterate(FILE* file, PropertyNode* propertyNode, int indentation)
{
	if (propertyNode->isStatic())
	{
		writeStringToFile("static ", file, indentation);
		indentation = 0;
	}
	writeStringToFile("::paf::Iterator* iterate_", file, indentation);
	writeStringToFile(propertyNode->m_name->m_str.c_str(), file);
	writeStringToFile("();", file);
}

void HeaderFileGenerator::generateCode_Property(FILE* file, PropertyNode* propertyNode, int indentation)
{
	TokenNode* firstToken;
	if (0 != propertyNode->m_filterNode)
	{
		firstToken = propertyNode->m_filterNode;
	}
	else if (0 != propertyNode->m_modifier)
	{
		firstToken = propertyNode->m_modifier;
	}
	else if (0 != propertyNode->m_typeName)
	{
		if (propertyNode->m_typeName->m_keyword)
		{
			firstToken = propertyNode->m_typeName->m_keyword;
		}
		else
		{
			firstToken = propertyNode->m_typeName->m_scopeNameList->m_scopeName->m_name;
		}
	}
	else
	{
		firstToken = propertyNode->m_name;
	}

	g_compiler.outputEmbededCodes(file, firstToken);
	if (propertyNode->isNoCode() || propertyNode->m_nativeName)
	{
		file = 0;
	}

	if(0 != propertyNode->m_get && 0 != propertyNode->m_set && propertyNode->m_get->m_keyword->m_tokenNo > propertyNode->m_set->m_keyword->m_tokenNo)
	{
		generateCode_Property_Set(file, propertyNode, indentation);
		writeStringToFile("\n", file);	
		generateCode_Property_Get(file, propertyNode, indentation);
	}
	else
	{
		if(0 != propertyNode->m_get)
		{
			generateCode_Property_Get(file, propertyNode, indentation);
			if(0 != propertyNode->m_set)
			{
				writeStringToFile("\n", file);
			}
		}
		if(0 != propertyNode->m_set)
		{
			generateCode_Property_Set(file, propertyNode, indentation);
		}
		g_compiler.outputEmbededCodes(file, propertyNode->m_name); //¸ñÊ½

	}	
	if (propertyNode->isArray())
	{
		writeStringToFile("\n", file);
		generateCode_Property_Size(file, propertyNode, indentation);
	}
	else if (propertyNode->isCollection())
	{
		writeStringToFile("\n", file);
		generateCode_Property_Iterate(file, propertyNode, indentation);
	}
};

void HeaderFileGenerator::generateCode_Method(FILE* file, MethodNode* methodNode, int indentation)
{
	TokenNode* firstToken;
	if (0 != methodNode->m_filterNode)
	{
		firstToken = methodNode->m_filterNode;
	}
	else if (0 != methodNode->m_modifier)
	{
		firstToken = methodNode->m_modifier;
	}
	else if (0 != methodNode->m_voidResult)
	{
		firstToken = methodNode->m_voidResult;
	}
	else if (0 != methodNode->m_resultTypeName)
	{
		if (methodNode->m_resultTypeName->m_keyword)
		{
			firstToken = methodNode->m_resultTypeName->m_keyword;
		}
		else
		{
			firstToken = methodNode->m_resultTypeName->m_scopeNameList->m_scopeName->m_name;
		}
	}
	else
	{
		firstToken = methodNode->m_name;
	}

	g_compiler.outputEmbededCodes(file, firstToken);
	if (methodNode->isNoCode() || methodNode->m_nativeName)
	{
		file = 0;
	}

	ClassNode* classNode = static_cast<ClassNode*>(methodNode->m_enclosing);
	if(classNode->isAdditionalMethod(methodNode))
	{
		writeStringToFile("\n", file);
	}
	if (0 != methodNode->m_modifier)
	{
		generateCode_Token(file, methodNode->m_modifier, indentation);
		indentation = 0;
	}
	if (0 != methodNode->m_voidResult)
	{
		generateCode_Token(file, methodNode->m_voidResult, indentation);
		indentation = 0;
	}
	if(0 != methodNode->m_resultTypeName)
	{
		generateCode_CompoundTypeName(file, methodNode->m_resultTypeName, methodNode->m_resultTypeCompound, methodNode->m_enclosing, true, indentation);
		indentation = 0;
		writeSpaceToFile(file);
	}
	//generateCode_Name(file, methodNode->m_name, methodNode->m_nativeName, indentation);
	generateCode_Identify(file, methodNode->m_name, indentation);
	generateCode_Token(file, methodNode->m_leftParenthesis, 0);
	std::vector<std::pair<TokenNode*, ParameterNode*>> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t parameterCount = parameterNodes.size();
	for(size_t i = 0; i < parameterCount; ++i)
	{
		if(parameterNodes[i].first)
		{
			generateCode_Token(file, parameterNodes[i].first, 0);
		}
		generateCode_Parameter(file, parameterNodes[i].second, classNode);
	}
	generateCode_Token(file, methodNode->m_rightParenthesis, 0);
	if(methodNode->m_constant)
	{
		generateCode_Token(file, methodNode->m_constant, 0);
	}
	if(methodNode->isAbstract())
	{
		writeStringToFile(" = 0 ", file);
	}
	generateCode_Token(file, methodNode->m_semicolon, 0);
}

