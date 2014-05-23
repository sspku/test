#include "FishingLayer.h"


#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#define SOUND_COIN        "Audio/sound_coin.ogg"
#else
#define SOUND_COIN        "Audio/sound_coin.mp3"
#endif // CC_PLATFOR_ANDROID

// android effect only support ogg
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#define SOUND_FIRE        "Audio/sound_fire.ogg"
#else
#define SOUND_FIRE        "Audio/sound_fire.mp3"
#endif // CC_PLATFOR_ANDROID



bool FishingLayer::init(){
	setTag(101);
	//将炮弹初始化为空，默认为空
	_bullet=NULL;
	//添加背景
	auto background = Sprite::create("CocoStudioRes/background.jpg");
	//设置锚点
	background->setAnchorPoint(Point(0, 0));
	//设置图片位置
	background->setPosition(Point(0, 0));
	//给背景加上标签
	background->setTag(102);
	//将背景精灵加入到捕鱼层
	addChild(background, 0);

	//读取 CocoStudio json 文件，构建的场景图, 并且添加到捕鱼场景中
	widget = dynamic_cast<Layout*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("CocoStudioRes/FishJoyMini_1.json"));
	addChild(widget, 2);


	/*--------------------------向SpriteFrameCache中添加文件--------------------------*/
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("CocoStudioRes/cannon-hd.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("GameScene/Item-chaojiwuqi-iphone-hd.plist");
	/*--------------------------取得一个炮台实例--------------------------*/
	cannon = dynamic_cast<ImageView*>(widget->getChildByName("cannon"));


	//获得船桨，并且使得它能够滚动
	auto ui_box_01_01 = dynamic_cast<ImageView*>(widget->getChildByName("ui_box_01_01"));
	ui_box_01_01->runAction(RepeatForever::create((RotateBy::create(3, 360, 360))));


	//获得得分标识的实例
	scoreLabel = dynamic_cast<TextAtlas*>(widget->getChildByName("Score"));
	score = 0;


	fishInital();

	/*-------------给组件设置可以触发点击事件-------------*/
	widget->setTouchEnabled(true);

	/*--------------------------给组件设置点击事件的回调函数--------------------------*/
	widget->addTouchEventListener(this, toucheventselector(FishingLayer::shootEvent));//需要添加射击事件

	//Get the pause buton and its touch event callback function
	auto pauseBtn = dynamic_cast<Button*>(widget->getChildByName("pauseBtn"));
	pauseBtn->addTouchEventListener(this, toucheventselector(FishingLayer::pauseEvent));//需要写一个暂停事件,暂停事件进入了一个新的暂停层

	//turn on the background music
	auto turnOnMusicBtn = dynamic_cast<ImageView *>(widget->getChildByName("ImageView_42"));
	turnOnMusicBtn->addTouchEventListener(this, toucheventselector(FishingLayer::turnOnMusic));//需要写一个开音乐事件
	//turn off the background music
	auto turnOffMusicBtn = dynamic_cast<Button *>(widget->getChildByName("music"));
	turnOffMusicBtn->addTouchEventListener(this, toucheventselector(FishingLayer::turnOffMusic));//需要写一个关音乐事件



	scheduleUpdate();
	return true;

}
/***********************射击事件方法*********************/
void FishingLayer::shootEvent(Widget* target, TouchEventType type){
	//传入的参数是目标组件，和触发事件的类型
	/***********************刚刚点击时*********************/
	if (type == TouchEventType::TOUCH_EVENT_BEGAN){

		//变成长炮筒
		cannon->loadTexture("actor_cannon1_72.png", UI_TEX_TYPE_PLIST);

	}
	/******************************点击结束时********************/
	else if (type == TouchEventType::TOUCH_EVENT_ENDED){

	   //变成原始炮筒图片
		cannon->loadTexture("actor_cannon1_71.png", UI_TEX_TYPE_PLIST);

		//改变炮台方向，getTouchEndPos是事件发生结束时，所在的位置
		FishingLayer::setCannonRotation(target, target->getTouchEndPos());//必须要先添加设置炮台转动方法setCannonRotation

		//发射炮弹
		bulletEndPosition = target->getTouchEndPos();
		bulletShoot(target->getTouchEndPos());//需要定义子弹射击方法，将子弹发射到目标位置，以及渔网的散开等。


	}
	/*************************鼠标按着移动时***********************/
	else if (type == TouchEventType::TOUCH_EVENT_MOVED){

		//改变炮台方向，getTouchMovePos是事件发生移动时，那一时刻所在的位置
		FishingLayer::setCannonRotation(target, target->getTouchMovePos());
	}
}
/***********************设置炮台转动方法*********************/
void FishingLayer::setCannonRotation(Widget* target, Point targetPos){

	/*两个参数，目标组件，和所在的位置
	*atan2函数
	*返回给定的 X 及 Y 坐标值的反正切值。
	*反正切的角度值等于 X 轴正方向与通过原点和给定坐标点 (Y坐标， X坐标) 的射线之间的夹角。
	*结果以弧度表示并介于 -pi 到 pi 之间（不包括 -pi）。
	*若要用度表示反正切值，请将结果再乘以 180/3.14159。
	*/
	auto radian = atan2(targetPos.y - 21.6f, targetPos.x - 480.0f);
	auto inclination = radian * 180 / 3.14;
	auto rotation = -(inclination)+90;

	//Set the rotation range
	if (rotation <= 70 && rotation >= -70){

		cannon->setRotation(rotation);
	}
}
/***********************子弹射击事件*********************/
void FishingLayer::bulletShoot(Point endPosition){
	//如果没有炮弹
	if (_bullet == NULL){  //需要先在头文件定义子弹射击用到的属性和渔网

		//初始化炮弹
		auto bullet = Sprite::createWithSpriteFrameName("weapon_bullet_007.png");
		auto netFish = SpriteBatchNode::create("GameScene/bullet10-hd.png", 5);
		//addChild(netFish,1);
		//图片GameScene/bullet10-hd.png只是渔网的4分之一，4幅图片通过旋转不同的角度组成一张渔网
		net = Sprite::createWithTexture(netFish->getTexture(), Rect(0, 0, 80, 80));
		net1 = Sprite::createWithTexture(netFish->getTexture(), Rect(0, 0, 80, 80));
		net2 = Sprite::createWithTexture(netFish->getTexture(), Rect(0, 0, 80, 80));
		net3 = Sprite::createWithTexture(netFish->getTexture(), Rect(0, 0, 80, 80));

		net1->setRotation(90.0f);
		net2->setRotation(180.0f);
		net3->setRotation(270.0f);

		float shifting;

		//Set the offest of the raotation
		if (cannon->getRotation() <= 0){

			shifting = 20.0f;
		}
		else{

			shifting = -20.0f;
		}

		//Set the anchorpoint, rotation, position of the bullet
		_bullet = bullet;
		_bullet->setAnchorPoint(Point(0.5, 0.5));
		//炮弹的方向与炮筒的方向一致
		_bullet->setRotation(cannon->getRotation());
		//炮弹的初始位置
		_bullet->setPosition(Point(cannon->getPosition().x - shifting, cannon->getPosition().y + 20));

		auto scale0 = ScaleTo::create(0.5, 0.3);
		auto scale1 = ScaleTo::create(0.5, 0.1);
		auto scale2 = ScaleTo::create(0.5, 0.35);
		auto scale3 = ScaleTo::create(0.5, 0.15);
		auto scale4 = ScaleTo::create(0.1, 0);
		auto scale00 = ScaleTo::create(0.5, 0.3);
		auto scale01 = ScaleTo::create(0.5, 0.1);
		auto scale02 = ScaleTo::create(0.5, 0.35);
		auto scale03 = ScaleTo::create(0.5, 0.15);
		auto scale04 = ScaleTo::create(0.1, 0);
		auto scale000 = ScaleTo::create(0.5, 0.3);
		auto scale001 = ScaleTo::create(0.5, 0.1);
		auto scale002 = ScaleTo::create(0.5, 0.35);
		auto scale003 = ScaleTo::create(0.5, 0.15);
		auto scale004 = ScaleTo::create(0.1, 0);
		auto scale0000 = ScaleTo::create(0.5, 0.3);
		auto scale0001 = ScaleTo::create(0.5, 0.1);
		auto scale0002 = ScaleTo::create(0.5, 0.35);
		auto scale0003 = ScaleTo::create(0.5, 0.15);
		auto scale0004 = ScaleTo::create(0.1, 0);
		net->setAnchorPoint(Point(0, 0));
		net1->setAnchorPoint(Point(0, 0));
		net2->setAnchorPoint(Point(0, 0));
		net3->setAnchorPoint(Point(0, 0));

		//Shoot the bullet and release after the action ended
		_bullet->runAction(Sequence::create(MoveTo::create(1, endPosition),
			CallFunc::create(CC_CALLBACK_0(FishingLayer::bulletRelease, this)),//需要定义子弹释放方法bulletRelease
			NULL));
		addChild(_bullet, 1);
		net->runAction(Sequence::create(scale0, scale1, scale2, scale3, scale4, NULL));
		net1->runAction(Sequence::create(scale00, scale01, scale02, scale03, scale04, NULL));
		net2->runAction(Sequence::create(scale000, scale001, scale002, scale003, scale004, NULL));
		net3->runAction(Sequence::create(scale0000, scale0001, scale0002, scale0003, scale0004, NULL));

		//Play bullet shoot music effect
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(SOUND_FIRE);//需要导入声音引擎文件
	}
}
/***********************子弹射击完成之后*********************/
void FishingLayer::bulletRelease(){

	//删除炮弹
	if (_bullet != NULL){
		_bullet->removeFromParent();
		_bullet = NULL;
	}
	//设置渔网出现位置
	net->setPosition(bulletEndPosition);
	net1->setPosition(bulletEndPosition);
	net2->setPosition(bulletEndPosition);
	net3->setPosition(bulletEndPosition);
	addChild(net, 1);
	addChild(net1, 1);
	addChild(net2, 1);
	addChild(net3, 1);
}

/***********************暂停事件*********************/
void FishingLayer::pauseEvent(Widget* target, TouchEventType type){

	if (type == TouchEventType::TOUCH_EVENT_ENDED){

		//Get the windows size of fishlayer
		auto winSize = Director::getInstance()->getWinSize();

		//Pause all the actions and animations
		this->onExit();

		//Get the background ant change it to the pause texture
		auto background = (Sprite*)getChildByTag(102);
		background->setTexture("GameScene/bgblur01_01-hd.png");
		background->setScaleX(winSize.width / background->getContentSize().width);
		background->setScaleY(winSize.height / background->getContentSize().height);
		background->setZOrder(2);

		//创建暂停层，需要定义暂停层的类，然后倒入进来。(去掉这句话，和头文件，就可以不用这个功能了）
		auto pauseLayer = FishingPauseLayer::create();
		this->getParent()->addChild(pauseLayer, 3);
	}
}


/***********************关闭音乐*********************/
void FishingLayer::turnOffMusic(Widget* target, TouchEventType type)
{
	//CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic("Audio/music_1.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}


/***********************打开音乐*********************/
void FishingLayer::turnOnMusic(Widget* target, TouchEventType type){

	CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}







//初始化鱼
void FishingLayer::fishInital(){
	Size visibleSize = Director::getInstance()->getVisibleSize();
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("MainMenu/FishActor-Small-hd.plist");
	 fish =Sprite::createWithSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_actor_001.png"));
	addChild(fish, 10);
	Size size = fish->getContentSize();
	fish->setPosition(Point(visibleSize.width - size.width, visibleSize.height - visibleSize.height / 2));
	auto moveAction1 = MoveTo::create(10, Point(size.width, fish->getPositionY()));
	auto moveAction2 = MoveTo::create(10, Point(visibleSize.width - size.width, fish->getPositionY()));
	auto CallFun = CallFunc::create(CC_CALLBACK_0(FishingLayer::turnBack, this, fish));
	auto seq = Sequence::create(
		moveAction1, CallFun,
		moveAction2, CallFun,
		NULL);
	auto repeat = RepeatForever::create(seq);
	fish->runAction(repeat);
	//Read the swimming animations textures
	auto fishes = Vector<SpriteFrame*>();
	fishes.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_actor_001.png"));
	fishes.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_actor_002.png"));
	fishes.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_actor_003.png"));
	fishes.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_actor_004.png"));
	//Create swimming animation
	auto fishAnimation = Animation::createWithSpriteFrames(fishes, 0.1);
	auto fishAnimate = Animate::create(fishAnimation);
	fish->runAction(RepeatForever::create(fishAnimate));
}

void FishingLayer::collideCheck(){

	auto deathFrames = Vector<SpriteFrame*>();
	deathFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_death_001.png"));
	deathFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_death_002.png"));
	deathFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_death_003.png"));
	deathFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_death_004.png"));
	deathFrames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("SmallFish_death_005.png"));

	//Create the death anmation
	auto deathAnimation = Animation::createWithSpriteFrames(deathFrames, 0.1);
	auto deathAnimate = Animate::create(deathAnimation);
	
	if (_bullet != NULL){
		if (_bullet->getBoundingBox().intersectsRect(fish->getBoundingBox())) {
			net->setPosition(Point(fish->getPositionX(), fish->getPositionY()));
			net1->setPosition(Point(fish->getPositionX(), fish->getPositionY()));
			net2->setPosition(Point(fish->getPositionX(), fish->getPositionY()));
			net3->setPosition(Point(fish->getPositionX(), fish->getPositionY()));
			addChild(net, 1);
			addChild(net1, 1);
			addChild(net2, 1);
			addChild(net3, 1);
			//Release the bullet
			_bullet->removeFromParent();
			_bullet = NULL;
			fish->runAction(deathAnimate);
			//fish->removeFromParent();
			//Set the score
			score += 1;
			char temp[64];
			sprintf(temp, "%d", score);
			scoreLabel->setStringValue(temp);
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(SOUND_COIN);
		}
	}
}
void FishingLayer::update(float delta){
	//Check the collide
	collideCheck();
}

void FishingLayer::turnBack(Node* sender){
	if (sender->getRotation() == 0.0f){
		sender->setRotation(180.00f);
	}
	else {
		sender->setRotation(0.00f);
	}
}