// Just to check that everyrhing can be compliled

template <class T>
class list {
};


/////////////////
// DirGraph

template <typename C>
class Vertex { public:
    typedef typename C::Edge Edge;
    // functionality

protected:
    list<Edge*> edges;
};

template <typename C>
class Edge { public:
   typedef typename C::Vertex Vertex;
    // functionality

protected:
    Vertex* first;
    Vertex* second;
};

template <typename C>
class Graph { public:
    typedef typename C::Vertex Vertex;
    typedef typename C::Edge Edge;
    // functionality and algorithms

protected:
    list<Vertex> vertices;
    list<Edge> edges;
};

struct GraphConfig {
    typedef Vertex<GraphConfig> Vertex;
    typedef Edge<GraphConfig> Edge;
};

Graph<GraphConfig> graph;


/////////////////
// DirGraph

template <typename C>
class DirVertex: public Vertex<C> { public:
    //{} type \Cls{Edge} is inherited
    //{} extended functionality

protected:
    // a new internal staff
};

template <typename C>
class DirEdge: public Edge<C> { public:
    //{} type \Cls{Vertex} is inherited

    Vertex* source() const { return m_first; }
    Vertex* target() const { return m_second; }
    // more functionality

protected:
    // a new internal staff
};

template <typename C>
class DirGraph: public Graph<C> { public:
    //{} types \Cls{Vertex} and \Cls{Edge} are inherited
    //{} new functionality and algorithms

protected:
    // a new internal staff
};

struct DirGraphConfig {
    typedef DirVertex<DirGraphConfig> Vertex;
    typedef DirEdge<DirGraphConfig> Edge;
};

DirGraph<DirGraphConfig> dirGraph;

///////////////////////////////
// Face

template <typename C>
class FaceEdge : public DirEdge<C> { public:
    //{} role \Cls{Vertex} is inherited
    typedef typename C::Face Face;

    Face* getFace() const { return face; }
    // more functionality

protected:
    Face* face;
};

template <typename C>
class Face { public:
    typedef typename C::Edge Edge;
    // functionality and algorithms

protected:
    list<Edge*> edges;
};

template <typename C>
class FaceGraph: public DirGraph<C> { public:
    //{} roles \Cls{Vertex} and \Cls{Edge} are inherited
    typedef typename C::Face Face;
    // new functionality and algorithms

protected:
    list<Face> faces;
};

struct FaceGraphConfig {
    typedef DirVertex<DirGraphConfig> Vertex;
    typedef FaceEdge<DirGraphConfig> Edge;
    typedef Face<DirGraphConfig> Face;
};

FaceGraph<FaceGraphConfig> faceGraph;
Face<DirGraphConfig> aFace;

void main()
{
}