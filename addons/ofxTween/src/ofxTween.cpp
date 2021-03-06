#include "ofxTween.h"

ofxTween::ofxTween():frameBased(false){
	duration = 0;
	completed = true;
#ifdef OFXTWEEN_USE_DELEGATE
	easingFunction = NULL;
#endif
	running = false;


	easing = NULL;
	id = -1;
	frameBased = false;
	tweening = false;
}

ofxTween::ofxTween(int id,  ofxEasing & easing, ofxEasingType type,  float from, float to, unsigned duration, unsigned delay) {
    ofxTween();
    setParameters(id, easing, type, from,to,duration,delay);
}

ofxTween::~ofxTween() {
#ifdef OFXTWEEN_USE_DELEGATE
	if(easingFunction) delete easingFunction;
#endif
}

void ofxTween::setParameters( ofxEasing & easing, ofxEasingType type, float from, float to, unsigned duration, unsigned delay){
	setParameters(id, easing, type, from,to,duration,delay);
}

void ofxTween::setParameters(int _id,  ofxEasing & _easing, ofxEasingType _type,  float _from, float _to, unsigned _duration, unsigned _delay){
	from.clear();
	to.clear();
	change.clear();
	pTarget.clear();

	id 		= _id;
	type 	= _type;
	easing 	= &_easing;
#ifdef OFXTWEEN_USE_DELEGATE
	if(easingFunction) delete easingFunction;
	switch(type){
	case easeIn:
		easingFunction = new ofxTweenDelegate(easing, &ofxEasing::easeIn);
	break;
	case easeOut:
		easingFunction = new ofxTweenDelegate(easing, &ofxEasing::easeOut);
	break;
	case easeInOut:
		easingFunction = new ofxTweenDelegate(easing, &ofxEasing::easeInOut);
	break;
	}
#endif
	addValue(_from,_to);
	if(frameBased){
		duration = _duration;
		delay = _delay;
	}else{
		duration = _duration*1000;
		delay = _delay*1000;
	}
	start();

	completed = false;
	running = true;
	tweening = false;
}

void ofxTween::addValue(float _from, float _to){
	from.push_back(_from);
	to.push_back(_to);
	change.push_back(_to-_from);
	pTarget.push_back(_from);
}

void ofxTween::start(){
	if(!frameBased){
		timestamp = Poco::Timestamp();
		timestamp += delay;
	}else{
		elapsed=0;
	}
}

void ofxTween::setDuration(int _duration) {
	if(frameBased){
		duration = _duration;
	}else{
		duration = _duration*1000;
	}
}

int ofxTween::getDuration() {
	return duration;
}
float ofxTween::getTo(unsigned pos){
	if(to.size()>pos)
		return to[pos];
	else
		return 0;
}
float ofxTween::getTarget(unsigned pos) {
	if(pTarget.size()>pos)
		return pTarget[pos];
	else
		return 0;
}

bool ofxTween::isRunning() {
	return running;
}

bool ofxTween::isCompleted() {
	return completed;
}

bool ofxTween::isTweening() {
	return tweening;
}

float ofxTween::update() {
    if(!completed){
        

		if(frameBased){
            ofxEasingArgs args;
			elapsed++;
			args.t= elapsed;
			args.d= float(duration);
			for(unsigned i=0; i<from.size(); i++){
				args.b=from[i];
				args.c=change[i];
#ifdef OFXTWEEN_USE_DELEGATE
				easingFunction->notify(this,args);
#else
				applyEasing(args);
#endif
				pTarget[i] = args.res;
			}
			if(pTarget[0]==to[0])
				running=false;
			else
				running = true;
		}else{
			if (timestamp.isElapsed(duration)){
				for(unsigned i=0; i<from.size(); i++){
					pTarget[i] = to[i];
				}
				running = false;
				completed = true;
				tweening = false;
				ofNotifyEvent(end_E,id);
			}

			else if(timestamp.elapsed()>0){
				if(!tweening)ofNotifyEvent(start_E, id);
				ofxEasingArgs args;
				float elapsedTime = float(timestamp.elapsed());
				args.t= elapsedTime;
				args.d= float(duration);
				for(unsigned i=0; i<from.size(); i++){
					args.b=from[i];
					args.c=change[i];

#ifdef OFXTWEEN_USE_DELEGATE
					easingFunction->notify(this,args);
#else
					applyEasing(args);
#endif
				pTarget[i] = args.res;
				}
				running = true;
				tweening = true;
			}
		}
	}
	return getTarget(0);
}


void ofxTween::setFrameBasedAnimation(bool frameBased){
	this->frameBased = frameBased;
}

float ofxTween::map(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp, ofxEasing & easing)
{
	return ofxTween::map(value, inputMin, inputMax, outputMin, outputMax, clamp, easing, ofxTween::easeInOut);
}

float ofxTween::map(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp, ofxEasing & easing, ofxEasingType type)
{
	ofxEasingArgs args;
	if(clamp){
		value = ofClamp(value, inputMin, inputMax);
	}
	args.t = value - inputMin;
	args.c = outputMax - outputMin;
	args.d = inputMax - inputMin;
	args.b = outputMin;
	
	switch (type) {
		case ofxTween::easeIn:
		{
			easing.easeIn(args);
			break;
		}
		case ofxTween::easeOut:
		{
			easing.easeOut(args);
			break;
		}
		default:
			easing.easeInOut(args);
	}
	
	return args.res;
}
#ifndef OFXTWEEN_USE_DELEGATE
void ofxTween::applyEasing(ofxEasingArgs& args){
	if (easing) {
	switch (type) {
		case ofxTween::easeIn:
		{
			easing->easeIn(args);
			break;
		}
		case ofxTween::easeOut:
		{
			easing->easeOut(args);
			break;
		}
		default:
			easing->easeInOut(args);
	}
}
}
#endif



