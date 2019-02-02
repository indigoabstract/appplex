// https://www.shadertoy.com/view/4sXSDf
#define PI 3.1415926535897
#define C  32.703
#define CS 34.648
#define D  36.708
#define DS 38.891
#define E  41.203
#define F  43.654
#define FS 46.249
#define G  48.999
#define GS 51.913
#define A  55.0
#define AS 58.270
#define B  61.735

vec2 notes[100];


void song(void) {
	// x = note, y = trigger time
    notes[ 0]=vec2(C      , 0.0);
    notes[ 1]=vec2(D      , 1.0);
    notes[ 2]=vec2(E      , 2.0);
    notes[ 3]=vec2(C      , 3.0);
    notes[ 4]=vec2(C      , 4.0);
    notes[ 5]=vec2(D      , 5.0);
    notes[ 6]=vec2(E      , 6.0);
    notes[ 7]=vec2(C      , 7.0);
    notes[ 8]=vec2(E      , 8.0);
    notes[ 9]=vec2(F      , 9.0);
    notes[10]=vec2(G      ,10.0);
    notes[11]=vec2(E      ,12.0);
    notes[12]=vec2(F      ,13.0);
    notes[13]=vec2(G      ,14.0);
    notes[14]=vec2(G      ,16.0);
    notes[15]=vec2(A      ,16.8);
    notes[16]=vec2(G      ,17.0);
    notes[17]=vec2(F      ,17.5);
    notes[18]=vec2(E      ,18.0);
    notes[19]=vec2(C      ,19.0);
    notes[20]=vec2(G      ,20.0);
    notes[21]=vec2(A      ,20.8);
    notes[22]=vec2(G      ,21.0);
    notes[23]=vec2(F      ,21.5);
    notes[24]=vec2(E      ,22.0);
    notes[25]=vec2(C      ,23.0);
    notes[26]=vec2(C      ,24.0);
    notes[27]=vec2(G*0.5  ,25.0);
    notes[28]=vec2(C      ,26.0);
    notes[29]=vec2(C      ,28.0);
    notes[30]=vec2(G*0.5  ,29.0);
    notes[31]=vec2(C      ,30.0);
}

vec2 getnotes(float time) 
{
	float w=0.;
	vec2 note=vec2(0.);
    for(int i=0;i<32;i++) {
		vec2 n=notes[i];
        if (n.y-time<0. && n.x>0.) { // if the note has to be played already
    	 	note.x=n.x; //save note
       	 	note.y=time-n.y; //save elapsed time

        }
    }
	return note;
}


void main(void)
{
	song(); //load notes
    vec2 uv = gl_FragCoord.xy / iResolution.xy-.5;
	uv.x*=iResolution.x/iResolution.y;
    float t=iGlobalTime*2.-2.;
    vec4 col=vec4(0.);
	vec2 note=getnotes(t+uv.x);
    col+=max(0.,.01-abs(uv.x))/.01*(.5+exp(-6.*note.y)*step(0.,t));
    col+=pow(max(0.,.02-abs(-.7+uv.y+note.x*.02))/.02,.3)*exp(-4.*note.y);
    gl_FragColor = col;
}
