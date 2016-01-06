#include "MetaPrimitive.h"
#include "Skeleton.h"

const string MetaPrimitive::type = "base";

MetaPrimitive::MetaPrimitive():skeleton(NULL)
{
}

MetaPrimitive::~MetaPrimitive()
{
}

ostream& operator<<(std::ostream&os, const MetaPrimitive& b){
	b.write(os);
	return os;
}

ifstream &operator>>(ifstream &stream, MetaPrimitive& ob){
	ob.read(stream);
	return stream;
}