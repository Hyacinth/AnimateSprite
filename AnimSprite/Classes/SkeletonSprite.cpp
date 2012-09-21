#include "SkeletonSprite.h"

USING_NS_CC;
//--------------------------------------------------------------------------------------------------
#define KEYFRAME_SCALE 1<<0
#define KEYFRAME_ROTATION 1<<1
#define KEYFRAME_TRANSLATE 1<<2
#define KEYFRAME_VISIBLE 1<<3
#define KEYFRAME_ALPHA 1<<4
#define KEYFRAME_SPRFRMIDX 1<<5
//--------------------------------------------------------------------------------------------------
class SpritePart : public cocos2d::CCObject{
public:
  int mask;
  unsigned int sprIdx;
  
  CCPoint scale;
  CCPoint translate;
  float rotation;
  bool visible;
  unsigned int sprFrmIdx;
  unsigned char alpha;
};
//--------------------------------------------------------------------------------------------------
struct Keyframe {
  ~Keyframe( void ){
    int count = this->spriteParts.size();
    for( int i=0;i<count;i++){
      SpritePart * spritePart =  this->spriteParts[i];
      CC_SAFE_RELEASE( spritePart );
    }
    this->spriteParts.clear();
  }
  float time;
  std::vector<SpritePart*> spriteParts;
};
//--------------------------------------------------------------------------------------------------
struct Timeline{
  ~Timeline( void ){
    int count = this->keyFrames.size();
    for( int i=0;i<count;i++){
      Keyframe * keyframe =  this->keyFrames[i];
      delete keyframe;
    }
    this->keyFrames.clear();
  }
  std::string name;
  bool isLoop;
  std::vector<Keyframe*> keyFrames;
};
//--------------------------------------------------------------------------------------------------
SkeletonSprite * SkeletonSprite::create( const char *anmsprFileName ){
  SkeletonSprite * sprite = new SkeletonSprite();
  if( sprite && sprite->init( anmsprFileName ) )
    sprite->autorelease();
  else
    CC_SAFE_DELETE( sprite );
  return sprite;
}
//--------------------------------------------------------------------------------------------------
SkeletonSprite::~SkeletonSprite( void ){
  int count = this->spriteFrames.size();
  for( int i=0;i<count;i++)
    CC_SAFE_RELEASE( this->spriteFrames[i] );
  this->spriteFrames.clear();
  std::map<unsigned int, CCSprite*>::iterator iter = this->sprites.begin();
  while( iter != this->sprites.end() ){
    CC_SAFE_RELEASE( iter->second );
    iter++;
  }
  this->sprites.clear();
  for( std::map<std::string, Timeline*>::iterator iter = this->animateTimeLines.begin();
      iter != this->animateTimeLines.end();iter++){
    Timeline * timeline = iter->second;
    delete timeline;
  }
  animateTimeLines.clear();
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::initSpriteSkeleton( CCNode * parentNode, tinyxml2::XMLElement * node ){
  tinyxml2::XMLElement * spriteEle = node->FirstChildElement( "SpriteNode" );
  while( spriteEle ){
    int depth = spriteEle->IntAttribute( "depth" );
    unsigned int index =spriteEle->UnsignedAttribute( "index" );
    unsigned int sprFrmIdx = spriteEle->UnsignedAttribute( "sprFrmIdx" );
    CCPoint anchor = ccp( spriteEle->FloatAttribute( "anchorX" ),
                         spriteEle->FloatAttribute( "anchorY" ) );
    CCPoint pos = ccp( spriteEle->FloatAttribute( "posX" ),
                      spriteEle->FloatAttribute( "posY" ) );
    CCSprite * sprite = CCSprite::createWithSpriteFrame( this->spriteFrames[sprFrmIdx] );
    sprite->retain();
    sprite->setPosition( pos );
    sprite->setAnchorPoint( anchor );
    this->sprites.insert( std::make_pair( index, sprite ));
    parentNode->addChild( sprite, depth );
    initSpriteSkeleton( sprite, spriteEle );
    spriteEle = spriteEle->NextSiblingElement( "SpriteNode" );
  }
}
//--------------------------------------------------------------------------------------------------
void processSpritePart( SpritePart * spritePart, tinyxml2::XMLElement * sprPartEle ){
  spritePart->sprIdx = sprPartEle->IntAttribute( "sprIdx" );
  spritePart->mask = 0;

  float scaleX = 0.0f;
  float scaleY = 0.0f;
  if( tinyxml2::XML_NO_ERROR == sprPartEle->QueryFloatAttribute( "scaleX" , &scaleX ) &&
     tinyxml2::XML_NO_ERROR == sprPartEle->QueryFloatAttribute( "scaleY" , &scaleY ) ){
    spritePart->mask |= KEYFRAME_SCALE;
    spritePart->scale = ccp( scaleX, scaleY );
  }

  float translateX = 0.0f;
  float translateY = 0.0f;
  if( tinyxml2::XML_NO_ERROR == sprPartEle->QueryFloatAttribute( "translateX" , &translateX ) &&
     tinyxml2::XML_NO_ERROR == sprPartEle->QueryFloatAttribute( "translateY" , &translateY ) ){
    spritePart->mask |= KEYFRAME_TRANSLATE;
    spritePart->translate = ccp( translateX, translateY );
  }

  float rotation = 0.0f;
  if( tinyxml2::XML_NO_ERROR == sprPartEle->QueryFloatAttribute( "rotation", &rotation ) ){
    spritePart->mask |= KEYFRAME_ROTATION;
    spritePart->rotation = rotation;
  }

  bool visible = true;
  if( tinyxml2::XML_NO_ERROR == sprPartEle->QueryBoolAttribute( "visible", &visible ) ){
    spritePart->mask |= KEYFRAME_VISIBLE;
    spritePart->visible = visible;
  }

  unsigned int sprFrmIdx = 0;
  if( tinyxml2::XML_NO_ERROR == sprPartEle->QueryUnsignedAttribute( "sprFrmIdx", &sprFrmIdx ) ){
    spritePart->mask |= KEYFRAME_SPRFRMIDX;
    spritePart->sprFrmIdx = sprFrmIdx;
  }

  unsigned int alpha = 0;
  if( tinyxml2::XML_NO_ERROR == sprPartEle->QueryUnsignedAttribute( "alpha", &alpha ) ){
    spritePart->mask |= KEYFRAME_ALPHA;
    spritePart->alpha = alpha;
  }
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::initAnimations(tinyxml2::XMLElement * node){
  tinyxml2::XMLElement * clipEle = node->FirstChildElement( "Clip" );
  while( clipEle ){
    Timeline * timeline = new Timeline();
    timeline->name = clipEle->Attribute( "name" );
    timeline->isLoop = false;
    clipEle->QueryBoolAttribute( "isLoop", &( timeline->isLoop ) );
    
    tinyxml2::XMLElement * keyframeEle = clipEle->FirstChildElement( "Keyframe" );
    while ( keyframeEle ) {
      Keyframe * keyframe = new Keyframe();
      keyframe->time = keyframeEle->FloatAttribute( "time" );
      tinyxml2::XMLElement * sprPartEle = keyframeEle->FirstChildElement( "SpritePart" );
      while ( sprPartEle ) {
        SpritePart * spritePart = new SpritePart();
        spritePart->autorelease();
        spritePart->retain();
        processSpritePart( spritePart, sprPartEle );
        keyframe->spriteParts.push_back( spritePart );
        sprPartEle = sprPartEle->NextSiblingElement( "SpritePart" );
      }
      timeline->keyFrames.push_back( keyframe );
      keyframeEle = keyframeEle->NextSiblingElement( "Keyframe" );
    }
    this->animateTimeLines.insert( std::make_pair( timeline->name, timeline ) );
    clipEle = clipEle->NextSiblingElement( "Timeline" );
  }
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::initSpriteLibrary( tinyxml2::XMLElement * spriteLibraryEle ){
  CCSpriteFrameCache * spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
  tinyxml2::XMLElement * atlasEle = spriteLibraryEle->FirstChildElement( "Atlas" );
  while( atlasEle ){
    const char * atlasFileName = atlasEle->Attribute( "src" );
    if( atlasFileName )
      spriteFrameCache->addSpriteFramesWithFile( atlasFileName );

    tinyxml2::XMLElement * spriteFrameEle = atlasEle->FirstChildElement( "SpriteFrame" );
    while( spriteFrameEle ){
      const char * spriteFrameName = spriteFrameEle->Attribute( "src" );
      if( spriteFrameName ){
        CCSpriteFrame * spriteFrame = spriteFrameCache->spriteFrameByName( spriteFrameName );
        spriteFrame->retain();
        this->spriteFrames.push_back( spriteFrame );
      }
      spriteFrameEle = spriteFrameEle->NextSiblingElement( "SpriteFrame" );
    }
    atlasEle = atlasEle->NextSiblingElement( "Atlas" );
  }
}
//--------------------------------------------------------------------------------------------------
bool SkeletonSprite::init( const char * anmsprFileName ){
  bool ret = false;
  do{
    CCFileUtils * fileUtils = CCFileUtils::sharedFileUtils();
    unsigned long size;
    const char * path = fileUtils->fullPathFromRelativePath( anmsprFileName );
    const unsigned char *xml = fileUtils->getFileData( path, "r", &size );
    tinyxml2::XMLDocument doc;
    int error = doc.Parse( (const char *)xml, size );
    CC_BREAK_IF( error );
    tinyxml2::XMLElement * rootEle = doc.FirstChildElement("AnimateSprite");
    CC_BREAK_IF( !rootEle );

    tinyxml2::XMLElement * spriteLibraryEle = rootEle->FirstChildElement("SpriteLibrary");
    CC_BREAK_IF( !spriteLibraryEle );
    this->initSpriteLibrary( spriteLibraryEle );
    
    tinyxml2::XMLElement * skeletonEle = rootEle->FirstChildElement("Skeleton");
    CC_BREAK_IF( !skeletonEle );
    this->initSpriteSkeleton( this, skeletonEle );
    
    tinyxml2::XMLElement * animationsEle = rootEle->FirstChildElement("Animations");
    CC_BREAK_IF( !animationsEle );
    this->initAnimations( animationsEle );
    
    ret = true;
  }while( 0 );
  return ret;
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::setSpriteAction( SpritePart * spritePart, float deltaTime ){
  CCSprite * sprite = this->sprites.at(spritePart->sprIdx);
  CCArray * actionArray = CCArray::create();
  //sprite->stopAllActions();
  if( spritePart->mask & KEYFRAME_SCALE )
    actionArray->addObject( CCScaleTo::create( deltaTime, spritePart->scale.x, spritePart->scale.y ) );
  if( spritePart->mask & KEYFRAME_ROTATION )
    actionArray->addObject( CCRotateTo::create( deltaTime, spritePart->rotation ) );
  if( spritePart->mask & KEYFRAME_TRANSLATE )
    actionArray->addObject( CCMoveTo::create( deltaTime, spritePart->translate ) );
  if( spritePart->mask & KEYFRAME_ALPHA )
    actionArray->addObject( CCFadeTo::create( deltaTime, spritePart->alpha ) );
  if( spritePart->mask & KEYFRAME_VISIBLE ){
    CCDelayTime *delay = CCDelayTime::create( deltaTime );
    if( spritePart->visible )
      actionArray->addObject( CCSequence::create( delay, CCShow::create() ) );
    else
      actionArray->addObject( CCSequence::create( delay, CCHide::create() ) );
  }
  if( spritePart->mask & KEYFRAME_SPRFRMIDX ){
    SEL_CallFuncO select = callfuncO_selector( SkeletonSprite::changeSpriteFrame );
    CCCallFuncO * func = CCCallFuncO::create( this, select, spritePart );
    CCDelayTime *delay = CCDelayTime::create( deltaTime );
    actionArray->addObject( CCSequence::create( delay, func ) );
  }
  sprite->runAction( CCSpawn::create( actionArray ) );
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::resetSprite( SpritePart * spritePart ){
  CCSprite * sprite = this->sprites.at(spritePart->sprIdx);
  sprite->stopAllActions();
  if( spritePart->mask & KEYFRAME_SCALE ){
    sprite->setScaleX(spritePart->scale.x);
    sprite->setScaleY(spritePart->scale.y);
  }
  if( spritePart->mask & KEYFRAME_ROTATION )
    sprite->setRotation( spritePart->rotation );
  if( spritePart->mask & KEYFRAME_TRANSLATE )
    sprite->setPosition( spritePart->translate );
  if( spritePart->mask & KEYFRAME_ALPHA )
    sprite->setOpacity( spritePart->alpha );
  if( spritePart->mask & KEYFRAME_VISIBLE )
    sprite->setVisible( spritePart->visible );
  if( spritePart->mask & KEYFRAME_SPRFRMIDX )
    sprite->initWithSpriteFrame( this->spriteFrames[ spritePart->sprFrmIdx ] );
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::performNext( void ){
  if( this->keyFrameCount >= (this->currentTimeline->keyFrames.size() - 1) ){
    if( this->currentTimeline->isLoop )
      this->keyFrameCount = 0;
    else
      return;
  }

  Keyframe * currentKeyFrame = this->currentTimeline->keyFrames[this->keyFrameCount];
  Keyframe * nextKeyFrame = this->currentTimeline->keyFrames[this->keyFrameCount+1];
  float deltaTime = nextKeyFrame->time - currentKeyFrame->time;
  for( int i=0;i<nextKeyFrame->spriteParts.size();i++){
    SpritePart * spritePart = nextKeyFrame->spriteParts[i];
    if( this->keyFrameCount == 0 ){
      SpritePart * currentSpritePart = currentKeyFrame->spriteParts[i];
      this->resetSprite( currentSpritePart );
    }
    this->setSpriteAction( spritePart, deltaTime );
  }
  this->keyFrameCount++;
  CCDelayTime * delay = CCDelayTime::create( deltaTime );
  SEL_CallFunc func = callfunc_selector( SkeletonSprite::performNext);
  CCCallFunc * callFunc = CCCallFunc::create( this, func );
  this->runAction( CCSequence::create( delay, callFunc ) );
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::perform( const std::string & name ){
  std::map<std::string,Timeline*>::iterator iter = animateTimeLines.find( name );
  if( iter == animateTimeLines.end() )
    return;
  this->currentTimeline = iter->second;
  this->keyFrameCount = 0;
  this->performNext();
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::changeSpriteFrame( CCObject * obj ){
  SpritePart * spritePart = (SpritePart*)obj;
  CCSpriteFrame * nextSpriteFrame = this->spriteFrames[ spritePart->sprFrmIdx ];
  CCSprite * sprite = this->sprites.at(spritePart->sprIdx);
  sprite->initWithSpriteFrame( nextSpriteFrame );  
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::stop( void ){
  std::map<unsigned int, CCSprite*>::iterator iter = this->sprites.begin();
  while( iter != this->sprites.end() ){
    iter->second->stopAllActions();
    iter++;
  }
  this->stopAllActions();
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::pause( void ){
  std::map<unsigned int, CCSprite*>::iterator iter = this->sprites.begin();
  while( iter != this->sprites.end() ){
    iter->second->pauseSchedulerAndActions();
    iter++;
  }
  this->pauseSchedulerAndActions();
}
//--------------------------------------------------------------------------------------------------
void SkeletonSprite::resume( void ){
  std::map<unsigned int, CCSprite*>::iterator iter = this->sprites.begin();
  while( iter != this->sprites.end() ){
    iter->second->resumeSchedulerAndActions();
    iter++;
  }
  this->resumeSchedulerAndActions();
}
//--------------------------------------------------------------------------------------------------