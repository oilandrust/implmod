#ifndef BVH_H
#define BVH_H

#include <vector>
#include <set>
#include <CGLA/Vec3f.h>
using namespace CGLA;
using namespace std;

class MetaPrimitive;

#define MAX_BVH_D 4

struct AABB{
	Vec3f bottomCorner;
	Vec3f topCorner;
	set<MetaPrimitive*> primitives;
};

struct AABBLite{
	Vec3f bottomCorner;
	Vec3f topCorner;
	MetaPrimitive* primitive;
	AABBLite():primitive(0){
	}
	AABBLite(const AABBLite& ab):bottomCorner(ab.bottomCorner),topCorner(ab.topCorner),primitive(ab.primitive){
	}
	AABBLite & operator= (const AABBLite & other)
    {
		this->bottomCorner = other.bottomCorner;
		this->topCorner = other.topCorner;
		this->primitive = other.primitive;
        // by convention, always return *this
        return *this;
    }
};

void renderAABB(const AABB& aabb);

class CompactBvh{
	public :
		int nbOfLeafs;
		int maxNbOfNonLeafNodes ;
		int maxNbOfNodes;
		int dataSize;
		Vec4f* data;
		CompactBvh();
		~CompactBvh();
		inline Vec4f nodeBottom(int node)const;
		inline Vec4f nodeTop(int node)const;
		inline void setNodeBottom(int node, const Vec4f& value);
		inline void setNodeTop(int node, const Vec4f& value);
		inline void setLeafPrimitives(int node, const Vec4f& spheres, const Vec4f& tubes);
		inline void setTags(int node, float tag);
		inline void setTag2(int node, float tag);

		bool Intersects(MetaPrimitive* prim, int node);
		bool Intersects(MetaPrimitive* prim,const AABBLite& bbox);


		void render(int level)const;
		void print()const;

		void initializeSplit(int node, vector<AABBLite>& aabbs);
		void buildNode(int node, vector<AABBLite>& primitives, const vector<MetaPrimitive*>& splitPrimitives);
		void buildNodeNoOverlap(int node, vector<AABBLite>& primitives, const vector<MetaPrimitive*>& splitPrimitives);
		void clear();
};


#endif