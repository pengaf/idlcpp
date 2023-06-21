#include "ParameterNode.h"
#include "TypeNameNode.h"
#include "IdentifyNode.h"
#include "MethodNode.h"
#include "ClassNode.h"
#include "RaiseError.h"
#include "TypeTree.h"
#include "Compiler.h"
#include "SourceFile.h"
#include <assert.h>


ParameterNode::ParameterNode(TypeNameNode* typeName, TypeCompound typeCompound, ParameterPassing passing, IdentifyNode* name)
{
	m_nodeType = snt_parameter;
	m_typeName = typeName;
	m_name = name;
	m_typeCompound = typeCompound;
	m_passing = passing;
	m_defaultDenote = nullptr;
	m_defaultParamCode = nullptr;
}

void ParameterNode::checkSemantic(TemplateArguments* templateArguments)
{
	if (m_defaultDenote)
	{
		SourceFile* sourceFile = GetCurrentSourceFile();
		m_defaultParamCode = sourceFile->getEmbededCode(m_defaultDenote->m_tokenNo + 1);
		if (nullptr == m_defaultParamCode)
		{
			RaiseError_MissingDefaultParameter(m_name);
		}
	}

	TypeNode* typeNode = m_typeName->getTypeNode(templateArguments);
	if(nullptr != typeNode)
	{
		g_compiler.useType(typeNode, templateArguments, (tc_none == m_typeCompound && pp_value == m_passing) ? tu_use_definition : tu_use_declaration, m_typeName);
	}
}
