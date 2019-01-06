#include "ofApp.h"


#ifndef __ARM_ARCH
    #define HOST "169.254.0.2"
#else
    #define HOST "localhost"
#endif

#define PORT 4444

#define USE_ARDUINO

#define USE_MASTERPLAN

#define BEATSEC 1
#define HARPSEC 2

#define FRYSEQ 2
#define TVSEQ 1

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("here 2018 ACME");
    
    //kicktoggle.smoothing( 25.0f );
    scene = 1;
    tvmax = 3;
    extracted.resize(4);
    for( int i=0; i<4; ++i){extracted[i] = i;}
    //-------------------GRAPHIC SETUP--------------
    gphase = 0.0;
    ofBackground(0);
    ofSetFrameRate(60);
    ofxGuiSetFillColor(ofColor(255, 80, 80));
    
    // --------------- PATCHING -----------------
    
    // ENGINE MASTER    
    engine.score.init( 4, 7, 160.0f); // sections, sequences, tempo
    engine.score.play();
    float o = -1.0f;   


    // --------------- MASTERPLAN --------------------------
    engine.score.sections[0].sequence(0).setLength( 1.0 );
    
    engine.score.sections[0].sequence(0).code = [&] () noexcept { // masterplan sequence
        pdsp::Sequence & plan = engine.score.sections[0].sequence(0); // defining variable into the lambda is fine

        // scene change
        if( plan.counter()==32 || (scene==0 && plan.counter()==16) ){ 
            #ifdef USE_MASTERPLAN
            int now = scene;
            now++;
            if(now>9){ now=0; }
            scene = now;
            //cout<<"masterplan change, scene"<<now<<"\n";
            #endif       
            plan.resetCount();      
        }
   
        switch(scene){            
            // ----------- FRY 
            case 0: 
                if( plan.counter() == 0) {
                    // drone 
                    drone_devil();
                    // harp
                    engine.score.sections[HARPSEC].launchCell(2, false); // off
                    // beat
                    engine.score.sections[BEATSEC].launchCell(FRYSEQ, false);
                }
            break;
            
            // ----------- TV VAR 1-2-3 cascaded
            case 1: 
                if( plan.counter() == 0) {   
                    tvmax = 1;
                    kicktrack(false);
                    drone_beer(8);
                    droneamp.getOFParameterFloat() = dB(-12.0f);
                    engine.score.sections[HARPSEC].launchCell(2, false); // off
                    engine.score.sections[BEATSEC].launchCell(TVSEQ, false);     
                }
            break;

            case 2: 
                if( plan.counter() == 0) { 
                    tvmax = 2;
                    kicktrack(false);  
                    drone_beer(8);
                    droneamp.getOFParameterFloat() = dB(-12.0f);
                    engine.score.sections[HARPSEC].launchCell(2, false); // off
                    engine.score.sections[BEATSEC].launchCell(TVSEQ, false);     
                }
            break;
            
            case 3: 
                if( plan.counter() == 0) {   
                    tvmax = 3;
                    kicktrack(true);
                    drone_beer(8);
                    droneamp.getOFParameterFloat() = dB(-12.0f);
                    engine.score.sections[HARPSEC].launchCell(2, false); // off
                    engine.score.sections[BEATSEC].launchCell(TVSEQ, false);     
                }
            break;
            
            // ----------- BEAT + DRONE
            case 4: // beat 
                if( plan.counter() == 0){
                    drone_main( 0.0f );
                    
                    mc0.regenerate = true;
                    engine.score.sections[HARPSEC].launchCell(2, false); // off
                    
                    engine.score.sections[BEATSEC].launchCell(3, false);
                }
            break;
            
            case 5: // beat + harp 
                if( plan.counter() == 0){
                    drone_main( 0.0f );
                    
                    mc1.regenerate = true;
                    mc1.steps = 32;
                    mc1.division = 4;
                    engine.score.sections[HARPSEC].launchCell(1, false); 
                    
                    engine.score.sections[BEATSEC].launchCell(4, false);
                }
                if( plan.counter()%8==0){
                    mc1.regenerate = true;
                }
            break;
            
            case 6: // beat + harp double
                if( plan.counter() == 0){
                    drone_main( 0.0f );
                    
                    mc1.regenerate = true;
                    mc1.steps = 32;
                    mc1.division = 8;
                    engine.score.sections[HARPSEC].launchCell(1, false); 
                    
                    engine.score.sections[BEATSEC].launchCell(5, false);
                }
                if( plan.counter()%28==0 ){
                    mc1.regenerate = true;
                }
            break;
            
            case 7:// beat + harp double
                if( plan.counter() == 0){
                    drone_main( 0.0f );
                    
                    mc0.regenerate = true;
                    mc0.steps = 32;
                    mc0.division = 8;
                    engine.score.sections[HARPSEC].launchCell(0, false); 
                    
                    engine.score.sections[BEATSEC].launchCell(5, false);
                }
                if( plan.counter()%28==0 ){
                    mc0.regenerate = true;
                }
            break;
            
            // ----------- HARP AMBIENT
            case 8: case 9:
                if( plan.counter() == 0) {   
                    // drone 
                    drone_randbeer(8);
                    droneamp.getOFParameterFloat() = 0.15f;
                    // harp
                    mc1.division = 8;
                    mc1.steps = 32;
                    engine.score.sections[HARPSEC].launchCell(1, false); // off
                    // beat
                    engine.score.sections[BEATSEC].launchCell(0, false);     
                }
                if( plan.counter()%4==0){
                    mc1.regenerate = true;
                }
            break;
        }
    };
    engine.score.sections[0].launchCell(0);

    // --------------- BEAT SEC ---------------------

    oscOut.openPort( HOST, PORT );    

    engine.score.sections[BEATSEC].sequence(0).set({ {0.0f, o}, {0.0f, o}, {0.0f, o}, {0.0f, o}, {0.0f, o}, {0.0f, o} }, 16.0f, 8.0f);
    
    // seq = randomly on tvs 
    engine.score.sections[BEATSEC].sequence(TVSEQ).code = [&] () noexcept {
        pdsp::Sequence & seq = engine.score.sections[BEATSEC].sequence(TVSEQ);
        seq.begin( 4.0, 4.0 );
            for( int i=0; i<25; ++i ){
                int a = rand()%4;
                int b = rand()%4;
                
                int temp = extracted[a];
                extracted[a] = extracted[b];
                extracted[b] = temp;
            }
        
            int activated = 0;
            for( int i=0; i<4; ++i){
                if( activated<tvmax ){
                    seq.message( 0.0, 1.0f, extracted[i] );
                    activated++;
                }else{
                    seq.message( 0.0, 0.0f, extracted[i] );
                }
            }
            seq.message( 0.0, 0.0f, 4);
            seq.message( 0.0, 0.0f, 5);
        seq.end();
    };
    
    // fry 
    engine.score.sections[BEATSEC].sequence(FRYSEQ).code = [&] () noexcept {
        pdsp::Sequence & seq = engine.score.sections[BEATSEC].sequence(FRYSEQ);
        seq.begin( 64.0, 1.0 );
            for( int i=0; i<5; ++i){
                seq.message( 0,  0.0f, i );
            }
            for( int i=0; i<64; ++i){
                seq.message( (double) i, (float)pdspChance(0.5f), 5 );
            }
        seq.end();
    };

    trackers.resize(3);
    for( size_t i=0; i<trackers.size(); ++i){
        trackers[i].load( ofToDataPath("tracker"+ofToString(i)+".seq") ); 
        engine.score.sections[BEATSEC].setCell( 3+i, &trackers[i], pdsp::Behavior::Loop );    
    }
          
    engine.score.sections[BEATSEC].launchCell(1);

    engine.score.sections[BEATSEC].out_message( 0 ) >> oscOut.address( "/relay/k1" );
    engine.score.sections[BEATSEC].out_message( 1 ) >> oscOut.address( "/relay/k2" );
    engine.score.sections[BEATSEC].out_message( 2 ) >> oscOut.address( "/relay/k3" );
    engine.score.sections[BEATSEC].out_message( 3 ) >> oscOut.address( "/relay/k4" );
    engine.score.sections[BEATSEC].out_message( 4 ) >> oscOut.address( "/relay/k5" );
    engine.score.sections[BEATSEC].out_message( 5 ) >> oscOut.address( "/relay/k6" );

    engine.score.sections[BEATSEC].out_trig( 0 ) >> kick.in("trig");    
    
    kick >> kicktoggle;
    kicktoggle >> engine.audio_out(0);
    kicktoggle >> engine.audio_out(1);

    beatsec.setup("panel", "beatsec.xml", 20, 20 );
    for(size_t i=0; i<trackers.size(); ++i){ 
        beatsec.add( trackers[i].parameters );
    }
    beatsec.add( kick.label( "kick" ) );
    beatsec.loadFromFile( ofToDataPath("beatsec.xml") );
    

    // ------------------ HARP SEC ------------------------
    
    engine.score.sections[HARPSEC].setCell( 0, &mc0, pdsp::Behavior::Loop ); 
    engine.score.sections[HARPSEC].setCell( 1, &mc1, pdsp::Behavior::Loop ); 

    engine.score.sections[HARPSEC].launchCell(1);
    mc0.load( ofToDataPath("mc0.chain") );
    mc1.load( ofToDataPath("mc1.chain") );

    ptable.setup(8, "ks pitches");
    ks.setup(8);
    for(int i=0; i<8; ++i){
        engine.score.sections[HARPSEC].out_trig(i) >> ks.in_trig(i);
        ptable.pitch[i] >> ks.in_pitch(i);
    }
       
    ks.out_L() >> engine.audio_out(0);
    ks.out_R() >> engine.audio_out(1);        

    harp.setup("panel", "harp.xml", 20+220, 20 );
    harp.add( mc0.parameters );
    harp.add( mc1.parameters );
    harp.add( ptable.parameters );
    harp.add( ks.parameters );
    harp.loadFromFile( ofToDataPath("harp.xml") );
    
    
    // ----------------- DRONE SETUP --------------------------------
    drone.setup( 420, 80 );
    drone.addSample( sample0 );
    drone.addSample( sample1 );
    drone.addSample( sample2 );
    drone.smoothing( 100.0f );

    poslfo >> drone.in_position();
    drone.out_L() >> tremL >> engine.audio_out(0);
    drone.out_R() >> tremR >> engine.audio_out(1);
    
    tremolo >> tremL.in_mod();
               tremR.in_mod();
    droneamp >> tremL.in_mod();
    droneamp >> tremR.in_mod();
    
    dronegui.setup( "dronegui", "graindrone.xml");
    dronegui.add( drone.parameters );
    dronegui.add( poslfo.label("position lfo") );
    dronegui.add( droneamp.set("drone amp", 1.0f, 0.0f, 1.0f));
    dronegui.add( tremolo.label("tremolo") );

    dronegui.setPosition( 20 + 220*2, 20 );
    dronegui.loadFromFile( ofToDataPath("graindrone.xml" ) );
   
    samples.setup( "samples", "samples.xml", 20 + 220*3, 20);   
    //samples.add( sample0.parameters );
    //samples.add( sample1.parameters );
    //samples.add( sample2.parameters );
    //samples.loadFromFile( "samples.xml");
    sample0.load(ofToDataPath("gridrideF.wav"));
    sample1.load(ofToDataPath("demons_noise.wav"));
    sample2.load(ofToDataPath("beer_16bit.wav"));
    
    // ----- setup arduino -------
#ifdef USE_ARDUINO
    arduino.connect("/dev/ttyACM0", 57600);
	ofAddListener(arduino.EInitialized, this, &ofApp::setupArduino);
	bSetupArduino = false;
#endif
    
    // preset ignore
    mc0.regenerate = true;   
    mc1.regenerate = true;   
    
    //----------------------AUDIO SETUP-------------
    engine.addExternalOut( oscOut ); // remember to add your OSC output to the engine
    engine.listDevices();

#ifdef __ARM_ARCH
    engine.setDeviceID(2); 
#else
    engine.setDeviceID(0); 
#endif
    engine.setup( 44100, 512, 3); 
    
}

//--------------------------------------------------------------
void ofApp::update(){
        
    gphase = engine.score.sections[BEATSEC].meter_playhead() * 0.125f;
        
        
#ifdef USE_ARDUINO
	arduino.update();    
	if (bSetupArduino) {
        
        int pwm = 201;
        
        switch ( scene ){
            case 0:
                pwm = ofMap( tremolo.meter_output(), -0.5f, 0.5f, 0, 255, true );
            break;
            
            case 8: case 9:
            {   
                float tri = fabs( ( gphase * 2.0f) - 1.0f );
                pwm = 1 + 200 * tri;
            }
            break;
        
            default: break;
        }
        lamp( pwm  ); 
        //lamp( 101 + 100 * sin(gphase*TWO_PI) ); 
    }
#endif

}

//--------------------------------------------------------------
void ofApp::draw(){
#ifndef __ARM_ARCH
    beatsec.draw();
    harp.draw();
    
    dronegui.draw();
    samples.draw();
    ofSetColor( 255, 80, 80 );
    drone.draw( 20 + 220*3, 500 );

    ofDrawBitmapString( ofToString( gphase ),20+220*3, 800 );
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){
        case 'q': scene = 1; engine.score.sections[0].launchCell(0, true); break;
        case 'w': scene = 4; engine.score.sections[0].launchCell(0, true); break;
        case 'e': scene = 5; engine.score.sections[0].launchCell(0, true); break;
        case 'r': scene = 6; engine.score.sections[0].launchCell(0, true); break;
        case 't': scene = 8; engine.score.sections[0].launchCell(0, true); break;
        case 'y': scene = 0; engine.score.sections[0].launchCell(0, true); break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------

void ofApp::drone_off(){
    drone.sample(0);
    drone.pitch( 0 );
    drone.positionControl.getOFParameterFloat() = 0.5f;
    poslfo.setAmount( 0.5f );
    poslfo.setDivide(4);
    poslfo.setPulse( 1);
    poslfo.setShape( 0 );
    droneamp.getOFParameterFloat() = 0.0f;
    tremolo.setAmount( 0.0f );
    tremolo.setDivide( 1 );                    
    tremolo.setPulse( 4 );
}

void ofApp::drone_devil(){
    drone.sample(1);
    drone.pitch( -12 );
    drone.positionControl.getOFParameterFloat() = 0.5f;
    poslfo.setAmount( 0.5f );
    poslfo.setDivide(4);
    poslfo.setPulse( 1);
    poslfo.setShape( 0 );
    droneamp.getOFParameterFloat() = 0.25f;
    tremolo.setAmount( 0.25f );
    tremolo.setDivide( 1 );                    
    tremolo.setPulse( 8 );
}

void ofApp::drone_randbeer( int bars ){
    drone.sample(2);
    drone.pitch( 0 );
    drone.positionControl.getOFParameterFloat() = 0.12f;
    poslfo.setAmount( 0.1f );
    poslfo.setDivide(bars);
    poslfo.setPulse( 1);
    poslfo.setShape( 4 );
    droneamp.getOFParameterFloat() = 1.0f;
    tremolo.setAmount( 0.0f );                        
}

void ofApp::drone_beer( int bars ){
    drone.sample(2);
    drone.pitch( 0 );
    drone.positionControl.getOFParameterFloat() = 0.12f;
    poslfo.setAmount( 0.1f );
    poslfo.setDivide(bars);
    poslfo.setPulse( 1);
    poslfo.setShape( 0 );
    droneamp.getOFParameterFloat() = 1.0f;
    tremolo.setAmount( 0.0f );                        
}

void ofApp::drone_main( float trem ){
    drone.sample(0);
    drone.pitch( -12 );
    drone.positionControl.getOFParameterFloat() = 0.5f;
    poslfo.setAmount( 0.4f );
    poslfo.setDivide( 8 );
    poslfo.setPulse( 1);
    poslfo.setShape( 0 );
    droneamp.getOFParameterFloat() = ofMap( trem, 0.0f, 1.0f, 1.0f, 0.5f, true);
    tremolo.setAmount( ofMap( trem, 0.0f, 1.0f, 0.0f, 0.5f, true) );     
}

//--------------------------------------------------------------
void ofApp::lamp( int fade ){
    fade = (fade>0) ? fade : 0; 
    fade = (fade<255) ? fade : 255; 
    arduino.sendPwm(9, fade);  
}

void ofApp::setupArduino(const int & version) {
	
	ofRemoveListener(arduino.EInitialized, this, &ofApp::setupArduino);
    bSetupArduino = true;
    // setup here
    arduino.sendDigitalPinMode(9, ARD_PWM);
    std::cout<<"[micro] arduino firmata activated\n";
}

void ofApp::kicktrack( bool active ){
    if(active){
        kicktoggle.setv(1.0f);
    }else{
        kicktoggle.setv(0.0f);
    }
}
