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
	//���ڵ���ʼ��Ϊ�գ�Ĭ��Ϊ��
	_bullet=NULL;
	//��ӱ���
	auto background = Sprite::create("CocoStudioRes/background.jpg");
	//����ê��
	background->setAnchorPoint(Point(0, 0));
	//����ͼƬλ��
	background->setPosition(Point(0, 0));
	//���������ϱ�ǩ
	background->setTag(102);
	//������������뵽�����
	addChild(background, 0);

	//��ȡ CocoStudio json �ļ��������ĳ���ͼ, ������ӵ����㳡����
	widget = dynamic_cast<Layout*>(cocostudio::GUIReader::getInstance()->widgetFromJsonFile("CocoStudioRes/FishJoyMini_1.json"));
	addChild(widget, 2);


	/*--------------------------��SpriteFrameCache������ļ�--------------------------*/
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("CocoStudioRes/cannon-hd.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("GameScene/Item-chaojiwuqi-iphone-hd.plist");
	/*--------------------------ȡ��һ����̨ʵ��--------------------------*/
	cannon = dynamic_cast<ImageView*>(widget->getChildByName("cannon"));


	//��ô���������ʹ�����ܹ�����
	auto ui_box_01_01 = dynamic_cast<ImageView*>(widget->getChildByName("ui_box_01_01"));
	ui_box_01_01->runAction(RepeatForever::create((RotateBy::create(3, 360, 360))));


	//��õ÷ֱ�ʶ��ʵ��
	scoreLabel = dynamic_cast<TextAtlas*>(widget->getChildByName("Score"));
	score = 0;


	fishInital();

	/*-------------��������ÿ��Դ�������¼�-------------*/
	widget->setTouchEnabled(true);

	/*--------------------------��������õ���¼��Ļص�����--------------------------*/
	widget->addTouchEventListener(this, toucheventselector(FishingLayer::shootEvent));//��Ҫ�������¼�

	//Get the pause buton and its touch event callback function
	auto pauseBtn = dynamic_cast<Button*>(widget->getChildByName("pauseBtn"));
	pauseBtn->addTouchEventListener(this, toucheventselector(FishingLayer::pauseEvent));//��Ҫдһ����ͣ�¼�,��ͣ�¼�������һ���µ���ͣ��

	//turn on the background music
	auto turnOnMusicBtn = dynamic_cast<ImageView *>(widget->getChildByName("ImageView_42"));
	turnOnMusicBtn->addTouchEventListener(this, toucheventselector(FishingLayer::turnOnMusic));//��Ҫдһ���������¼�
	//turn off the background music
	auto turnOffMusicBtn = dynamic_cast<Button *>(widget->getChildByName("music"));
	turnOffMusicBtn->addTouchEventListener(this, toucheventselector(FishingLayer::turnOffMusic));//��Ҫдһ���������¼�



	scheduleUpdate();
	return true;

}
/***********************����¼�����*********************/
void FishingLayer::shootEvent(Widget* target, TouchEventType type){
	//����Ĳ�����Ŀ��������ʹ����¼�������
	/***********************�ոյ��ʱ*********************/
	if (type == TouchEventType::TOUCH_EVENT_BEGAN){

		//��ɳ���Ͳ
		cannon->loadTexture("actor_cannon1_72.png", UI_TEX_TYPE_PLIST);

	}
	/******************************�������ʱ********************/
	else if (type == TouchEventType::TOUCH_EVENT_ENDED){

	   //���ԭʼ��ͲͼƬ
		cannon->loadTexture("actor_cannon1_71.png", UI_TEX_TYPE_PLIST);

		//�ı���̨����getTouchEndPos���¼���������ʱ�����ڵ�λ��
		FishingLayer::setCannonRotation(target, target->getTouchEndPos());//����Ҫ�����������̨ת������setCannonRotation

		//�����ڵ�
		bulletEndPosition = target->getTouchEndPos();
		bulletShoot(target->getTouchEndPos());//��Ҫ�����ӵ�������������ӵ����䵽Ŀ��λ�ã��Լ�������ɢ���ȡ�


	}
	/*************************��갴���ƶ�ʱ***********************/
	else if (type == TouchEventType::TOUCH_EVENT_MOVED){

		//�ı���̨����getTouchMovePos���¼������ƶ�ʱ����һʱ�����ڵ�λ��
		FishingLayer::setCannonRotation(target, target->getTouchMovePos());
	}
}
/***********************������̨ת������*********************/
void FishingLayer::setCannonRotation(Widget* target, Point targetPos){

	/*����������Ŀ������������ڵ�λ��
	*atan2����
	*���ظ����� X �� Y ����ֵ�ķ�����ֵ��
	*�����еĽǶ�ֵ���� X ����������ͨ��ԭ��͸�������� (Y���꣬ X����) ������֮��ļнǡ�
	*����Ի��ȱ�ʾ������ -pi �� pi ֮�䣨������ -pi����
	*��Ҫ�öȱ�ʾ������ֵ���뽫����ٳ��� 180/3.14159��
	*/
	auto radian = atan2(targetPos.y - 21.6f, targetPos.x - 480.0f);
	auto inclination = radian * 180 / 3.14;
	auto rotation = -(inclination)+90;

	//Set the rotation range
	if (rotation <= 70 && rotation >= -70){

		cannon->setRotation(rotation);
	}
}
/***********************�ӵ�����¼�*********************/
void FishingLayer::bulletShoot(Point endPosition){
	//���û���ڵ�
	if (_bullet == NULL){  //��Ҫ����ͷ�ļ������ӵ�����õ������Ժ�����

		//��ʼ���ڵ�
		auto bullet = Sprite::createWithSpriteFrameName("weapon_bullet_007.png");
		auto netFish = SpriteBatchNode::create("GameScene/bullet10-hd.png", 5);
		//addChild(netFish,1);
		//ͼƬGameScene/bullet10-hd.pngֻ��������4��֮һ��4��ͼƬͨ����ת��ͬ�ĽǶ����һ������
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
		//�ڵ��ķ�������Ͳ�ķ���һ��
		_bullet->setRotation(cannon->getRotation());
		//�ڵ��ĳ�ʼλ��
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
			CallFunc::create(CC_CALLBACK_0(FishingLayer::bulletRelease, this)),//��Ҫ�����ӵ��ͷŷ���bulletRelease
			NULL));
		addChild(_bullet, 1);
		net->runAction(Sequence::create(scale0, scale1, scale2, scale3, scale4, NULL));
		net1->runAction(Sequence::create(scale00, scale01, scale02, scale03, scale04, NULL));
		net2->runAction(Sequence::create(scale000, scale001, scale002, scale003, scale004, NULL));
		net3->runAction(Sequence::create(scale0000, scale0001, scale0002, scale0003, scale0004, NULL));

		//Play bullet shoot music effect
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(SOUND_FIRE);//��Ҫ�������������ļ�
	}
}
/***********************�ӵ�������֮��*********************/
void FishingLayer::bulletRelease(){

	//ɾ���ڵ�
	if (_bullet != NULL){
		_bullet->removeFromParent();
		_bullet = NULL;
	}
	//������������λ��
	net->setPosition(bulletEndPosition);
	net1->setPosition(bulletEndPosition);
	net2->setPosition(bulletEndPosition);
	net3->setPosition(bulletEndPosition);
	addChild(net, 1);
	addChild(net1, 1);
	addChild(net2, 1);
	addChild(net3, 1);
}

/***********************��ͣ�¼�*********************/
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

		//������ͣ�㣬��Ҫ������ͣ����࣬Ȼ���������(ȥ����仰����ͷ�ļ����Ϳ��Բ�����������ˣ�
		auto pauseLayer = FishingPauseLayer::create();
		this->getParent()->addChild(pauseLayer, 3);
	}
}


/***********************�ر�����*********************/
void FishingLayer::turnOffMusic(Widget* target, TouchEventType type)
{
	//CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic("Audio/music_1.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}


/***********************������*********************/
void FishingLayer::turnOnMusic(Widget* target, TouchEventType type){

	CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}







//��ʼ����
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