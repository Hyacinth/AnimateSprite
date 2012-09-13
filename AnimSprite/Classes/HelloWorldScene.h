#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class SkeletonSprite;

class HelloWorld : public cocos2d::CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommand to return the exactly class pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);

  void onEnterTransitionDidFinish( void );
  void onExitTransitionDidStart( void );
  
    // implement the "static node()" method manually
    CREATE_FUNC(HelloWorld);
private:
  SkeletonSprite * skeletonSpr;
};

#endif // __HELLOWORLD_SCENE_H__
