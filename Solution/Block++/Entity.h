#pragma once
#include "Node.h"
#include "Factory.h"

class Entity : public virtual Rath::Node, public ISaveable
{
public:
	UUID(1000)
};

