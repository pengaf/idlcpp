#include "ParameterNode.h"
#include "TypeNameNode.h"
#include "IdentifyNode.h"
#include "MethodNode.h"
#include "ClassNode.h"
#include "RaiseError.h"
#include "TypeTree.h"
#include "Compiler.h"
#include <assert.h>

ParameterNode::ParameterNode(TypeNameNode* typeName, TypeCompound typeCompound)
{
	m_nodeType = snt_parameter;
	m_typeName = typeName;
	m_name = nullptr;
	m_typeCompound = typeCompound;
	m_passing = pp_value;
	m_defaultDenote = nullptr;
}

void ParameterNode::checkSemantic(TemplateArguments* templateArguments)
{
	TypeNode* typeNode = m_typeName->getTypeNode(templateArguments);
	if(0 == typeNode)
	{
		return;
	}
	g_compiler.useType(typeNode, templateArguments, tc_none == m_typeCompound ? tu_use_definition : tu_use_declaration, m_typeName);
}
