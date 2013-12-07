#ifndef NDEBUG
# define NDEBUG
#endif

#include "libfidtrack_treeidmap.h"
#ifdef _MSC_VER
# pragma warning( disable: 4091)
#endif /* _MSC_VER */

#include <m_pd.h>

#include <map>
#include <vector>
#include <string>
#include <fstream>

#include <string.h>

static int find_maximum_tree_depth( const std::string& s )
{
    int result = 0;
    for( std::string::size_type i=1; i < s.size(); ++i ){ // skip first character which is the black/white flag

        int d = s[i] - '0';
        if( d > result )
            result = d;
    }

    return result;
}


static int find_maximum_descendent_count( const std::string& s )
{
    int result = 0;

    std::vector<int> adjacenciesAtLevel( s.size(), 0 );
    int currentLevel = 0;
    for( std::string::size_type i=1; i < s.size(); ++i ){ // skip first character which is the black/white flag

        int d = s[i] - '0';

        if( d > 0 ){

            adjacenciesAtLevel[d-1]++;
            if( adjacenciesAtLevel[d-1] > result )
                result = adjacenciesAtLevel[d-1];

        }

        if( d < currentLevel ){
            for( std::vector<int>::iterator j = adjacenciesAtLevel.begin() + d;
                    j != adjacenciesAtLevel.begin() + currentLevel; ++j ){

                *j = 0;
            }         
        }

        currentLevel = d;
    }

    return result;
}



class TreeIdMapImplementation{
    TreeIdMap* owner_;

    struct charstar_less{
        bool operator()( const char *lhs, const char *rhs ) const
        {
            return strcmp( lhs, rhs ) < 0;
        }
    };

    typedef std::map<const char*, int, charstar_less> map_type;
    map_type treeIdMap_;

    std::vector< const char *> strings_;

public:
    TreeIdMapImplementation( TreeIdMap* treeidmap, const char *file_name )
        : owner_( treeidmap )
    {

        int minNodeCount = 0;
        int maxNodeCount = 0;
        int minDepth = 0;
        int maxDepth = 0;
        int maxAdjacencies = 0;


        std::ifstream is( file_name );
        std::string s;
        int id = 0;

        if( !is.good() ){
            error("error opening configuration file");
        }else{

            minNodeCount = 0x7FFF;
            maxNodeCount = 0;

            minDepth = 0x7FFF;
            maxDepth = 0;

            maxAdjacencies = 0;
            
            while( !is.eof() ){
#ifdef _MSC_VER
	      std::string s;
#else
                s.clear();
#endif
                is >> s;
                if( s.empty() )
                    continue;

		int depthSequenceLength;

                // ensure that the depth sequence has a root colour prefix
                // of 'w' (white) or 'b' (black). if not, prepend one.
                if( s[0] == 'w' || s[0] == 'b' ){
                    depthSequenceLength = ( s.size() - 1 );
                }else{
                    depthSequenceLength = ( s.size() );
                    s = 'w' + s;
                }
                

                char *ss = new char[ s.size() + 1 ];
                strings_.push_back( ss );
                strcpy( ss, s.c_str() );
                std::pair<map_type::iterator, bool> i = treeIdMap_.insert( std::make_pair( ss, id++ ) );
                if( i.second ){

                    if( depthSequenceLength < minNodeCount )
                        minNodeCount = depthSequenceLength;
                    if( depthSequenceLength > maxNodeCount )
                        maxNodeCount = depthSequenceLength;

                    int maxTreeDepth = find_maximum_tree_depth( s );
                    
                    if( maxTreeDepth < minDepth )
                        minDepth = maxTreeDepth;
                    if( maxTreeDepth > maxDepth )
                        maxDepth = maxTreeDepth;

                    int maxNodeAdjacencies = find_maximum_descendent_count( s ) + 1;
                    if( maxNodeAdjacencies > maxAdjacencies )
                        maxAdjacencies = maxNodeAdjacencies;
                        
                }else{
					error("error inserting tree '%s' into map", s.c_str() );
                }
            }
        }

        owner_->tree_count = treeIdMap_.size();
        owner_->min_node_count = minNodeCount;
        owner_->max_node_count = maxNodeCount;
        owner_->min_depth = minDepth;
        owner_->max_depth = maxDepth;
        owner_->max_adjacencies = maxAdjacencies;
    }

    ~TreeIdMapImplementation()
    {
        while( !strings_.empty() ){
	  delete [] reinterpret_cast<const char*>(strings_.back());
            strings_.pop_back();
        }
    }

    int treestring_to_id( const char *treestring )
    {
        map_type::iterator i = treeIdMap_.find( treestring );
        if( i != treeIdMap_.end() )
            return i->second;
        else
            return INVALID_TREE_ID;
    }    
};

void initialize_treeidmap_from_file( TreeIdMap* treeidmap, const char *file_name )
{
    treeidmap->implementation_ = new TreeIdMapImplementation( treeidmap, file_name );
}


void terminate_treeidmap( TreeIdMap* treeidmap )
{
  delete static_cast<TreeIdMapImplementation*>(treeidmap->implementation_);
    treeidmap->implementation_ = 0;
}

// returns -1 for unfound id
int treestring_to_id( TreeIdMap* treeidmap, const char *treestring )
{
  return (static_cast<TreeIdMapImplementation*>(treeidmap->implementation_))->treestring_to_id( treestring );
}
