#include "VariableTypeNode.h"
#include "TypeNameNode.h"
#include "IdentifyNode.h"
#include "MethodNode.h"
#include "ClassNode.h"
#include "RaiseError.h"
#include "TypeTree.h"
#include "Compiler.h"
#include "SourceFile.h"
#include <assert.h>

VariableTypeNode::VariableTypeNode(TypeNameNode* typeName, TypeCompound typeCompound)
{
	m_nodeType = snt_variable_type;
	m_typeName = typeName;
	m_typeCompound = typeCompound;
}

void VariableTypeNode::checkSemantic(TemplateArguments* templateArguments, bool outputParam)
{
	TypeNode* typeNode = m_typeName->getTypeNode(templateArguments);
	if(nullptr != typeNode)
	{
		g_compiler.useType(typeNode, templateArguments, (tc_none == m_typeCompound && !outputParam) ? tu_use_definition : tu_use_declaration, m_typeName);
	}
}
