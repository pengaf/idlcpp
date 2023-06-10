#include "SourceFileGenerator.h"
#include "Utility.h"
#include "SourceFile.h"
#include "ProgramNode.h"
#include "NamespaceNode.h"
#include "TokenNode.h"
#include "IdentifyNode.h"
#include "EnumeratorListNode.h"
#include "ScopeNameListNode.h"
#include "MemberListNode.h"
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
#include "Options.h"
#include "Platform.h"
#include "CommonFuncs.h"
#include "TypeTree.h"
#include <assert.h>

void generateCode_Token(FILE* file, TokenNode* tokenNode, int indentation);
void generateCode_Identify(FILE* file, IdentifyNode* identifyNode, int indentation, bool addSpace = true);
void generateCode_Parameter(FILE* file, MethodNode* methodNode, ParameterNode* parameterNode, ScopeNode* scopeNode);
//void generateCode_CompoundTypeName(FILE* file, TypeNameNode* typeNameNode, TypeCompound typeCompound, ScopeNode* scopeNode, bool addKeyword, int indentation);
void generateCode_ParameterList(FILE* file, MethodNode* methodNode, ParameterListNode* parameterListNode, ScopeNode* scopeNode);
void writeInterfaceMethodImpl_AssignInputParam(ParameterNode* parameterNode, size_t argIndex, FILE* file, int indentation);
void writeInterfaceMethodImpl_SetOutputParamType(ParameterNode* parameterNode, size_t argIndex, FILE* file, int indentation);
void writeInterfaceMethodImpl_CastOutputParam(ParameterNode* parameterNode, size_t argIndex, FILE* file, int indentation);


/*
void writeDelegateImpl_InitResult(DelegateNode* delegateNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, delegateNode->m_resultTypeName, 0);
	const char* sign;
	if (delegateNode->byValue())
	{
		sign = "";
	}
	else
	{
		sign = "*";
	}
	sprintf_s(buf, "%s%s __res__;\n", typeName.c_str(), sign);
	writeStringToFile(buf, file, indentation);
}

void writeDelegateImpl_ReturnResult(DelegateNode* delegateNode, FILE* file, int indentation)
{
	if (delegateNode->byRef())
	{
		writeStringToFile("return *__res__;\n", file, indentation);
	}
	else
	{
		writeStringToFile("return __res__;\n", file, indentation);
	}
}

void writeDelegateImpl_CastResult(DelegateNode* delegateNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, delegateNode->m_resultTypeName, 0);

	TypeNameNode* resultNode = static_cast<TypeNameNode*>(delegateNode->m_resultTypeName);
	if (delegateNode->byValue())
	{
		switch (typeCategory)
		{
			//case void_type: impossible
		case primitive_type:
			sprintf_s(buf, "__result__.castToPrimitive(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &__res__);\n", typeName.c_str());
			break;
		case enum_type:
			sprintf_s(buf, "__result__.castToEnum(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &__res__);\n", typeName.c_str());
			break;
		case object_type:
			sprintf_s(buf, "__result__.castToValue(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &__res__);\n", typeName.c_str());
			break;
		case introspectable_type:
			sprintf_s(buf, "__result__.castToReference(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &__res__);\n", typeName.c_str());
			break;
		default:
			assert(false);
		}
		writeStringToFile(buf, file, indentation);
	}
	else
	{
		switch (typeCategory)
		{
		case void_type:
			sprintf_s(buf, "__result__.castToVoidPtr(&__res__);\n");
			break;
		case primitive_type:
			sprintf_s(buf, "__result__.castToPrimitivePtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&__res__);\n", typeName.c_str());
			break;
		case enum_type:
			sprintf_s(buf, "__result__.castToEnumPtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&__res__);\n", typeName.c_str());
			break;
		case object_type:
			sprintf_s(buf, "__result__.castToValuePtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&__res__);\n", typeName.c_str());
			break;
		case introspectable_type:
			sprintf_s(buf, "__result__.castToReferencePtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&__res__);\n", typeName.c_str());
			break;
		default:
			assert(false);
		}
		writeStringToFile(buf, file, indentation);
		if (delegateNode->byNew())
		{
			writeStringToFile("__result__.unhold();\n", file, indentation);
		}
	}
}

void writeDelegateImpl_SetResultType(DelegateNode* delegateNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, delegateNode->m_resultTypeName, 0);

	if (delegateNode->byValue() && (primitive_type == typeCategory || enum_type == typeCategory))
	{
		if (primitive_type == typeCategory)
		{
			sprintf_s(buf, "__result__.assignNullPrimitive(RuntimeTypeOf<%s>::RuntimeType::GetSingleton());\n", typeName.c_str());
		}
		else
		{
			sprintf_s(buf, "__result__.assignNullEnum(RuntimeTypeOf<%s>::RuntimeType::GetSingleton());\n", typeName.c_str());
		}
	}
	else
	{
		sprintf_s(buf, "__result__.assignNullPtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton());\n", typeName.c_str());
	}
	writeStringToFile(buf, file, indentation);
}
*/


void SourceFileGenerator::generateCode(FILE* dstFile, SourceFile* sourceFile, const char* fullPathName, const char* baseName)
{
	generateCode_Program(dstFile, sourceFile->m_syntaxTree, fullPathName, baseName);
}

void SourceFileGenerator::generateCode_Program(FILE* file, ProgramNode* programNode, const char* fileName, const char* cppName)
{
	char buf[4096];
	std::string pafcorePath;
	GetRelativePath(pafcorePath, fileName, g_options.m_pafcorePath.c_str());
	FormatPathForInclude(pafcorePath);

	writeStringToFile("#pragma once\n\n", file);

	sprintf_s(buf, "#include \"%s.h\"\n", cppName);
	writeStringToFile(buf, file);

	sprintf_s(buf, "#include \"%s%s\"\n", cppName, g_options.m_metaHeaderFilePostfix.c_str());
	writeStringToFile(buf, file);

	//sprintf_s(buf, "#include \"%sRefCountImpl.h\"\n\n", pafcorePath.c_str());
	//writeStringToFile(buf, file);

	generateCode_Namespace(file, programNode, -1);
}

void SourceFileGenerator::generateCode_Namespace(FILE* file, NamespaceNode* namespaceNode, int indentation)
{
	//if (namespaceNode->isNoCode())
	//{
	//	file = 0;
	//}

	char buf[4096];
	if(!namespaceNode->isGlobalNamespace())
	{
		sprintf_s(buf, "namespace %s\n", namespaceNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation);
		writeStringToFile("{\n\n", file, indentation);
	}
	std::vector<MemberNode*> memberNodes;
	namespaceNode->m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_namespace:
			generateCode_Namespace(file, static_cast<NamespaceNode*>(memberNode), indentation + 1);
			break;
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(memberNode), "", indentation + 1);
			break;
		//case snt_delegate:
		//	generateCode_Delegate(file, static_cast<DelegateNode*>(memberNode), namespaceNode, "", indentation + 1);
			break;
		}
	}

	if(!namespaceNode->isGlobalNamespace())
	{
		writeStringToFile("}\n\n", file, indentation);
	}
}

void GetClassName(std::string& className, ClassNode* classNode)
{
	className = classNode->m_name->m_str;
	if(classNode->m_templateParametersNode)
	{
		std::vector<IdentifyNode*> templateParameterNodes;
		classNode->m_templateParametersNode->collectParameterNodes(templateParameterNodes);
		className += "<";
		size_t count = templateParameterNodes.size();
		for(size_t i = 0; i < count; ++i)
		{
			if(0 != i)
			{
				className += ", ";
			}
			className += templateParameterNodes[i]->m_str;
		}
		className += ">";
	}
}

void SourceFileGenerator::generateCode_Class(FILE* file, ClassNode* classNode, const std::string& scopeClassName, int indentation)
{
	//if (classNode->isNoCode())
	//{
	//	file = 0;
	//}
	std::string typeName;
	GetClassName(typeName, classNode);
	typeName = scopeClassName + typeName;

	bool isInline = 0 != classNode->m_templateParametersNode;

	if (!(classNode->isNoCode() || classNode->isNoMeta()))
	{
		generateCode_TemplateHeader(file, classNode, indentation);
		if (isInline)
		{
			writeStringToFile("inline ::paf::ClassType* ", file, indentation);
		}
		else
		{
			writeStringToFile("::paf::ClassType* ", file, indentation);
		}
		writeStringToFile(typeName.c_str(), file);
		writeStringToFile("::GetType()\n", file);
		writeStringToFile("{\n", file, indentation);
		writeStringToFile("return ::RuntimeTypeOf<", file, indentation + 1);
		writeStringToFile(typeName.c_str(), file);
		writeStringToFile(">::RuntimeType::GetSingleton();\n", file);
		writeStringToFile("}\n\n", file, indentation);

		//if (classNode->isIntrospectable())
		//{
		//	generateCode_TemplateHeader(file, classNode, indentation);
		//	if (isInline)
		//	{
		//		writeStringToFile("inline ::paf::ClassType* ", file, indentation);
		//	}
		//	else
		//	{
		//		writeStringToFile("::paf::ClassType* ", file, indentation);
		//	}
		//	writeStringToFile(typeName.c_str(), file);
		//	writeStringToFile("::getType()\n", file);
		//	writeStringToFile("{\n", file, indentation);
		//	writeStringToFile("return ::RuntimeTypeOf<", file, indentation + 1);
		//	writeStringToFile(typeName.c_str(), file);
		//	writeStringToFile(">::RuntimeType::GetSingleton();\n", file);
		//	writeStringToFile("}\n\n", file, indentation);

		//	generateCode_TemplateHeader(file, classNode, indentation);
		//	if (isInline)
		//	{
		//		writeStringToFile("inline void* ", file, indentation);
		//	}
		//	else
		//	{
		//		writeStringToFile("void* ", file, indentation);
		//	}
		//	writeStringToFile(typeName.c_str(), file);
		//	writeStringToFile("::getAddress()\n", file);
		//	writeStringToFile("{\n", file, indentation);
		//	writeStringToFile("return this;\n", file, indentation + 1);
		//	writeStringToFile("}\n\n", file, indentation);
		//}
	}

	if (!classNode->isNoCode())
	{
		if (!classNode->m_additionalMethods.empty())
		{
			size_t count = classNode->m_additionalMethods.size();
			for (size_t i = 0; i < count; ++i)
			{
				generateCode_AdditionalMethod(file, classNode->m_additionalMethods[i], typeName, indentation);
			}
		}
	}


	typeName += "::";
	std::vector<MemberNode*> memberNodes;
	classNode->m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(memberNode), typeName, indentation);
			break;
		//case snt_delegate:
		//	generateCode_Delegate(file, static_cast<DelegateNode*>(memberNode), classNode, typeName, indentation);
			break;
		}
	}
}

/*
void SourceFileGenerator::generateCode_Delegate(FILE* file, DelegateNode* delegateNode, ScopeNode* scopeNode, const std::string& scopeClassName, int indentation)
{
	char buf[4096];
	if (delegateNode->isNoCode())
	{
		file = 0;
	}
	TypeNode* resultTypeNode = delegateNode->m_resultTypeName->getTypeNode(0);
	bool isVoid = (void_type == resultTypeNode->getTypeCategory(0) && 0 == delegateNode->m_passing);

	std::string typeName = scopeClassName + delegateNode->m_name->m_str;

	if (0 != delegateNode->m_resultConst)
	{
		generateCode_Token(file, delegateNode->m_resultConst, indentation);
		generateCode_TypeName(file, delegateNode->m_resultTypeName, delegateNode->m_enclosing, true, 0);
	}
	else
	{
		generateCode_TypeName(file, delegateNode->m_resultTypeName, delegateNode->m_enclosing, true, indentation);
	}
	if (0 != delegateNode->m_passing)
	{
		generateCode_Token(file, delegateNode->m_passing, 0);
	}
	writeSpaceToFile(file);
	writeStringToFile(typeName.c_str(), file);
	writeStringToFile("::invoke", file);
	generateCode_Token(file, delegateNode->m_leftParenthesis, 0);
	generateCode_ParameterList(file, delegateNode->m_parameterList, delegateNode->m_enclosing);
	generateCode_Token(file, delegateNode->m_rightParenthesis, 0);
	writeStringToFile("\n", file);
	writeStringToFile("{\n", file, indentation);

	std::vector<ParameterNode*> parameterNodes;
	delegateNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t paramCount = parameterNodes.size();

	writeStringToFile("bool __arguments_init__ = false;\n", file, indentation + 1);
	if (!isVoid)
	{
		writeStringToFile("bool __res_init__ = false;\n", file, indentation + 1);
		writeDelegateImpl_InitResult(delegateNode, file, indentation + 1);
	}
	
	writeStringToFile("::paf::Variant __result__;\n", file, indentation + 1);
	if (0 < paramCount)
	{
		sprintf_s(buf, "::paf::Variant __arguments__[%zd];\n", paramCount);
		writeStringToFile(buf, file, indentation + 1);
	}
	sprintf_s(buf, "::paf::Variant* __args__[%zd] = {0", paramCount + 1);
	writeStringToFile(buf, file, indentation + 1);
	for (size_t i = 0; i < paramCount; ++i)
	{
		sprintf_s(buf, ", &__arguments__[%zd]", i);
		writeStringToFile(buf, file, 0);
	}
	writeStringToFile("};\n", file, 0);

	writeStringToFile("::paf::CallBack* __callBack__ = getCallBack();\n", file, indentation + 1);
	writeStringToFile("while (__callBack__)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	writeStringToFile("::paf::CallBack* __next__ = __callBack__->getNext();\n", file, indentation + 2);
	writeStringToFile("::paf::ClassType* __classType__ = __callBack__->getType();\n", file, indentation + 2);
	writeStringToFile("if (__classType__ == ::paf::FunctionCallBack::GetType())\n", file, indentation + 2);
	writeStringToFile("{\n", file, indentation + 2);
	writeStringToFile("::paf::FunctionCallBack* __functionCallBack__ = static_cast<::paf::FunctionCallBack*>(__callBack__);\n", file, indentation + 3);
	writeStringToFile(isVoid ? "" : "__res__ = ", file, indentation + 3);
	writeStringToFile("(*(CallBackFunction)__functionCallBack__->m_function)(__functionCallBack__->m_userData", file, 0);
	for (size_t i = 0; i < paramCount; ++i)
	{
		sprintf_s(buf, ", %s", parameterNodes[i]->m_name->m_str.c_str());
		writeStringToFile(buf, file, 0);
	}
	writeStringToFile(");\n", file, 0);
	writeStringToFile("}\n", file, indentation + 2);

	writeStringToFile("else\n", file, indentation + 2);
	writeStringToFile("{\n", file, indentation + 2);

	writeStringToFile("if(!__arguments_init__)\n", file, indentation + 3);
	writeStringToFile("{\n", file, indentation + 3);
	writeStringToFile("__arguments_init__ = true;\n", file, indentation + 4);
	//if (!isVoid)
	//{
	//	writeDelegateImpl_SetResultType(delegateNode, file, indentation + 4);
	//}
	for (size_t i = 0; i < paramCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		if (parameterNode->isInput())
		{
			writeInterfaceMethodImpl_AssignInputParam(parameterNode, i, file, indentation + 4);
		}
		else
		{
			writeInterfaceMethodImpl_SetOutputParamType(parameterNode, i, file, indentation + 4);
		}
	}
	writeStringToFile("}\n", file, indentation + 3);

	sprintf_s(buf, "__callBack__->invoke(&__result__, __args__, %d);\n", paramCount + 1);
	writeStringToFile(buf, file, indentation + 3);

	if (!isVoid)
	{
		writeStringToFile("if(!__res_init__)\n", file, indentation + 3);
		writeStringToFile("{\n", file, indentation + 3);
		writeStringToFile("__res_init__ = false;\n", file, indentation + 4);
		for (size_t i = 0; i < paramCount; ++i)
		{
			ParameterNode* parameterNode = parameterNodes[i];
			if (parameterNode->isOutput())
			{
				writeInterfaceMethodImpl_CastOutputParam(parameterNode, i, file, indentation + 4);
			}
		}
		writeDelegateImpl_CastResult(delegateNode, file, indentation + 4);
		writeStringToFile("}\n", file, indentation + 3);
	}
	writeStringToFile("}\n", file, indentation + 2);
	writeStringToFile("__callBack__ = __next__;\n", file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);//while
	if (!isVoid)
	{
		writeStringToFile("return __res__;\n", file, indentation + 1);
	}
	writeStringToFile("}\n", file, indentation);
}
*/


void SourceFileGenerator::generateCode_TemplateHeader(FILE* file, ClassNode* classNode, int indentation)
{
	if(classNode->m_templateParametersNode)
	{
		std::vector<IdentifyNode*> templateParameterNodes;
		classNode->m_templateParametersNode->collectParameterNodes(templateParameterNodes);
		writeStringToFile("template<", file, indentation);
		size_t count = templateParameterNodes.size();
		for(size_t i = 0; i < count; ++i)
		{
			if(0 != i)
			{
				writeStringToFile(",", file);
			}
			writeStringToFile("typename ", file);
			writeStringToFile(templateParameterNodes[i]->m_str.c_str(), file);
		}
		writeStringToFile(">\n", file);
	}
}


void SourceFileGenerator::generateCode_AdditionalMethod(FILE* file, MethodNode* methodNode, const std::string& typeName, int indentation)
{
	char wrappedTypeName[4096];

	if (methodNode->isNoCode())
	{
		file = 0;
	}

	ClassNode* classNode = static_cast<ClassNode*>(methodNode->m_enclosing);
	if ("New" == methodNode->m_name->m_str)
	{
		sprintf_s(wrappedTypeName, "::paf::SharedPtr<%s>", typeName.c_str());
	}
	else
	{
		assert("NewArray" == methodNode->m_name->m_str);
		sprintf_s(wrappedTypeName, "::paf::SharedArray<%s>", typeName.c_str());
	}

	bool isInline = 0 != classNode->m_templateParametersNode;
	generateCode_TemplateHeader(file, classNode, indentation);

	if (isInline)
	{
		writeStringToFile("inline ", file, indentation);
		writeStringToFile(wrappedTypeName, file);
	}
	else
	{
		writeStringToFile(wrappedTypeName, file, indentation);
	}

	//if (0 != methodNode->m_passing)
	//{
	//	generateCode_Token(file, methodNode->m_passing, 0);
	//}

	writeSpaceToFile(file);
	writeStringToFile(typeName.c_str(), file);
	writeStringToFile("::", file);

	generateCode_Identify(file, methodNode->m_name, 0);

	generateCode_Token(file, methodNode->m_leftParenthesis, 0);
	std::vector<ParameterNode*> parameterNodes;

	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t parameterCount = parameterNodes.size();
	for (size_t i = 0; i < parameterCount; ++i)
	{
		if (0 != i)
		{
			writeStringToFile(", ", file);
		}
		generateCode_Parameter(file, methodNode, parameterNodes[i], classNode->m_enclosing);
	}
	generateCode_Token(file, methodNode->m_rightParenthesis, 0);
	writeStringToFile("\n", file);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("return ", file, indentation + 1);
	writeStringToFile(wrappedTypeName, file);
	writeStringToFile("::Make(", file);
	for (size_t i = 0; i < parameterCount; ++i)
	{
		if (i != 0)
		{
			writeStringToFile(", ", file);
		}
		writeStringToFile(parameterNodes[i]->m_name->m_str.c_str(), file);
	}
	writeStringToFile(");\n", file);
	
	writeStringToFile("}\n\n", file, indentation);
}