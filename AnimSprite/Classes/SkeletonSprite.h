#ifndef AnimSprite_SkeletonSprite_h
#define AnimSprite_SkeletonSprite_h
//--------------------------------------------------------------------------------------------------
#include "cocos2d.h"
#include "tinyxml2.h"
#include <vector>
#include <map>
struct Timeline;
class SpritePart;
//--------------------------------------------------------------------------------------------------
class SkeletonSprite : public cocos2d::CCNode{
public:
  static SkeletonSprite * create( const char * anmsprFileName );
  ~SkeletonSprite( void );
  
  void perform( const std::string & name );
  void stop( void );
  void pause( void );
  void resume( void );
  
private:
  std::vector<cocos2d::CCSpriteFrame*> spriteFrames;
  std::map<unsigned int, cocos2d::CCSprite*> sprites;
  std::map<std::string,Timeline*> animateTimeLines;
  
  Timeline * currentTimeline;
  int keyFrameCount;
  
  bool init( const char * anmsprFileName );
  
  void initSpriteLibrary( tinyxml2::XMLElement * spriteLibraryEle );
  void initSpriteSkeleton( cocos2d::CCNode * parentNode, tinyxml2::XMLElement * node );
  void initAnimations( tinyxml2::XMLElement * node );
  
  void changeSpriteFrame( cocos2d::CCObject * obj );
  void performNext( void );
  void resetSprite( SpritePart * spritePart );
  void setSpriteAction( SpritePart * spritePart ,float deltaTime );
};
//--------------------------------------------------------------------------------------------------
#endif
