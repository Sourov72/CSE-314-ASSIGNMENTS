#include<bits/stdc++.h>
#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>
#include<stdint.h>

using namespace std;


//semaphore and mutex control sleep and wake up
pthread_mutex_t kioskmtx, *beltmtx, speckioskmtx, vipleftmtx, viprightmtx, priorvipmtx, boardmtx, timemtx, globalboolmtx;
sem_t kiosksem, *beltsem, vipleftsem, viprightsem, specialkiosksem, boardsem;
int M, N, P, w, x, y, z;
int pid = 1;
int gtime = 0;

bool priorcheck = false;



class passenger{


    int id;
    bool vip = false;
    bool boardingpass = false;

public:

    void setId(int tid){
        id = tid;
    }

    void setVip(bool tvip){
        vip = tvip;
    }

    void setPass(bool pass){
        boardingpass = pass;
    }

   


    int getId(){
        return id;
    }

    bool getVip(){
       
        return vip;
    }

    bool getPass(){
        return boardingpass;
    }

    
};

queue<passenger> passengerq, *beltq, vipleftq, viprightq, specialkioskq, boardq;

void * create_passenger(void * arg){


    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(3);
    string s;


    while(1){
        // if(gtime > 100) break;
        passenger p;
        p.setId(pid);
        pid++;
        p.setVip(rand() % 2);
        p.setPass(false);
        pthread_mutex_lock(&kioskmtx);
        pthread_mutex_lock(&timemtx);
        passengerq.push(p);
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has arrived at airport at time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&kioskmtx);
        sem_post(&kiosksem);

        sleep(distribution(generator));
        // sleep(3);



    }
}


void * timecount(void * arg){


    while(1){

        sleep(1);
        pthread_mutex_lock(&timemtx);
        gtime++;
        pthread_mutex_unlock(&timemtx);


    }


}


void show_passenger(){

    while(passengerq.size()) {
      cout << passengerq.front().getId() << endl;
      passengerq.pop();
   }



}



void * checkinKiosk(void * arg){

    // int id = *((int*) arg);

    int id;
    id = (intptr_t) arg;

    // cout << "thread with " << id << " created" << endl;
    passenger p;
    string s;

    while(1){

        
        sem_wait(&kiosksem);
        pthread_mutex_lock(&kioskmtx);
		pthread_mutex_lock(&timemtx);
        // cout << "kiosk lock" << endl;
		p = passengerq.front();
        passengerq.pop();
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has started self-check in at kiosk " << id << " at time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);
		pthread_mutex_unlock(&kioskmtx); 
        // cout << "kiosk unlock" << endl;
        sleep(w);
        p.setPass(rand() % 2);

        pthread_mutex_lock(&timemtx);
        // cout << "kiosk lock" << endl;
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has finished check in at time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);
        // cout << "kiosk unlock" << endl;
        // printf("in thread %d, passenger id %d\n",id, p.getId());
        

        if(p.getVip()){

            pthread_mutex_lock(&vipleftmtx);
            pthread_mutex_lock(&globalboolmtx);
            if(vipleftq.size() == 0 && !priorcheck) {

                pthread_mutex_lock(&priorvipmtx);
                priorcheck = true;
            }
                
            pthread_mutex_unlock(&globalboolmtx);
            pthread_mutex_lock(&timemtx);
            // cout  << "passenger " << p.getId()  << "vip kiosk lock" << endl;
            
            vipleftq.push(p);
            

            cout << "passenger " << p.getId() << " (VIP) has started waiting in vip line from time " << gtime << endl;
            pthread_mutex_unlock(&timemtx);
            pthread_mutex_unlock(&vipleftmtx);
            sem_post(&vipleftsem);
            // cout  << "passenger " << p.getId()  << "vip kiosk unlock" << endl;

        }
        else{

            int randbelt = rand() % N;
            pthread_mutex_lock(&beltmtx[randbelt]);
            pthread_mutex_lock(&timemtx);
            // cout  << "passenger " << p.getId() << "rand belt kiosk lock" << endl;
            beltq[randbelt].push(p);
            
            cout << "passenger " << p.getId() << " has started waiting for security check in belt " << randbelt << " from time " << gtime << endl;
            pthread_mutex_unlock(&timemtx);
            pthread_mutex_unlock(&beltmtx[randbelt]);
            sem_post(&beltsem[randbelt]);
            //  cout  << "passenger " << p.getId()  << "rand belt kiosk unlock" << endl;

        }

    }

}




void * leftright(void * arg){

    passenger p;

    queue<passenger> tempsq, boardsq;
    string s;
    while(1){

        sem_wait(&vipleftsem);
        pthread_mutex_lock(&vipleftmtx);
        pthread_mutex_lock(&timemtx);
        int val = vipleftq.size();

        while(val > 0){

            p = vipleftq.front();
            vipleftq.pop();
            tempsq.push(p);
            boardsq.push(p);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started walking the vip line from left to right at time " << gtime << endl;

            if(val != 1)
                sem_wait(&vipleftsem);

            val--;



        }
         
        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&vipleftmtx);
        // cout << "leftrightbef" << endl;

        sleep(z);

        // cout << "leftrightaf" << endl;
        pthread_mutex_lock(&timemtx);
        // cout << "timemtx in vip left right locked" << endl;

        while(tempsq.size()){
            p = tempsq.front();
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has crossed the vip line from left to right at time " << gtime << endl;
            tempsq.pop();

        }
        
        if(vipleftq.size() == 0){
            pthread_mutex_lock(&globalboolmtx);
            pthread_mutex_unlock(&priorvipmtx);
            priorcheck = false;
            pthread_mutex_unlock(&globalboolmtx);
            
        }
            
        pthread_mutex_unlock(&timemtx);
        // cout << "timemtx in vip left right unlocked" << endl;
        
       

        

        while(boardsq.size()){


            pthread_mutex_lock(&boardmtx);
            pthread_mutex_lock(&timemtx);

            p = boardsq.front();
            boardsq.pop();
            boardq.push(p);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started waiting to be boarded at time " << gtime << endl;

            pthread_mutex_unlock(&timemtx);
            pthread_mutex_unlock(&boardmtx);
            sem_post(&boardsem);



        }





    }


}


void * rightleft(void * arg){



    passenger p;

    queue<passenger> tempsq, spboardsq;
    string s;
    while(1){

        sem_wait(&viprightsem);
        pthread_mutex_lock(&priorvipmtx);
        pthread_mutex_lock(&viprightmtx);
        pthread_mutex_lock(&timemtx);

        int val = viprightq.size();

        while(val > 0){

            p = viprightq.front();
            viprightq.pop();
            tempsq.push(p);
            spboardsq.push(p);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started walking the vip line from right to left at time " << gtime << endl;

            if(val != 1)
                sem_wait(&viprightsem);

            val--;



        }

        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&viprightmtx);
        pthread_mutex_unlock(&priorvipmtx);


        sleep(z);

        pthread_mutex_lock(&timemtx);

        while(tempsq.size()){
            p = tempsq.front();
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has crossed the vip line from right to left at time " << gtime << endl;
            tempsq.pop();

        }

        pthread_mutex_unlock(&timemtx);


        while(spboardsq.size()){


            pthread_mutex_lock(&speckioskmtx);
            pthread_mutex_lock(&timemtx);

            p = spboardsq.front();
            spboardsq.pop();
            specialkioskq.push(p);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started waiting at the special kiosk at time " << gtime << endl;

            pthread_mutex_unlock(&timemtx);
            pthread_mutex_unlock(&speckioskmtx);
            sem_post(&specialkiosksem);



        }


    }


}



void * boarding(void * arg){

    passenger p;
    string s;

    while(1){

        sem_wait(&boardsem);
        pthread_mutex_lock(&boardmtx);
        pthread_mutex_lock(&timemtx);

        p = boardq.front();
        boardq.pop();
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has started boarding the plane  at time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&boardmtx);

        sleep(y);


        if(p.getPass()){

            
            pthread_mutex_lock(&timemtx);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has boarded the plant at time " << gtime << endl;
            pthread_mutex_unlock(&timemtx);
        }

        else{

            pthread_mutex_lock(&viprightmtx);
            pthread_mutex_lock(&timemtx);
            viprightq.push(p);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started waiting in returned vip line from time " << gtime << endl;
            pthread_mutex_unlock(&timemtx);
            pthread_mutex_unlock(&viprightmtx);
            sem_post(&viprightsem);

        }



    }
    

}

void * returnback(void * arg){


    passenger p;
    string s;
    while(1){

        sem_wait(&specialkiosksem);

        pthread_mutex_lock(&speckioskmtx);
        pthread_mutex_lock(&timemtx);
        p = specialkioskq.front();
        specialkioskq.pop();
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has started check in at special kiosk from time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&speckioskmtx);
        p.setPass(rand() % 2);

        sleep(w);


        pthread_mutex_lock(&timemtx);
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has finished check in at special kiosk at time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);


        pthread_mutex_lock(&vipleftmtx);
        pthread_mutex_lock(&globalboolmtx);
        if(vipleftq.size() == 0 && !priorcheck) {
                pthread_mutex_lock(&priorvipmtx);
                priorcheck = true;
        }
            
        pthread_mutex_unlock(&globalboolmtx);
        pthread_mutex_lock(&timemtx);
        
        vipleftq.push(p);
        
        s =  p.getVip() ? "(VIP)": "";
        cout << "passenger " << p.getId() << s << " has started waiting in vip line for return passengers at time " << gtime << endl;
        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&vipleftmtx);
        sem_post(&vipleftsem);

    }


}




void * securityCheck(void * arg){

    int id;
    passenger p;
    queue<passenger> tempsq, boardsq;
    id = (intptr_t) arg;
    string s;
    while(1){

        
        sem_wait(&beltsem[id]);

        pthread_mutex_lock(&beltmtx[id]);
        pthread_mutex_lock(&timemtx);
        int val = min((int)beltq[id].size(), P);

        // cout << "value for security check: thread id: " << id << ", " << val << endl;
    
        while(val > 0){

            p = beltq[id].front();
            beltq[id].pop();
            tempsq.push(p);
            boardsq.push(p);
            // cout << "in security thread " << id << ", passenger id " << p.getId() << " checked in "<< endl;
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started the security check at time " << gtime << endl;

            if(val != 1)
                sem_wait(&beltsem[id]);



            val--;


        }
        pthread_mutex_unlock(&timemtx);
        pthread_mutex_unlock(&beltmtx[id]);
        // cout << "before secruity check" << endl;
        sleep(x);
        // cout << "after secrity check " << tempsq.size() << endl;

        pthread_mutex_lock(&timemtx);

        // cout << "timemtx in security locked" << endl;

        while(tempsq.size()){
            p = tempsq.front();
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has crossed the security check at time " << gtime << endl;
            tempsq.pop();


        }
        pthread_mutex_unlock(&timemtx);
        // cout << "timemtx in security check unlocked" << endl;


        while(boardsq.size()){


            pthread_mutex_lock(&boardmtx);
            pthread_mutex_lock(&timemtx);
            p = boardsq.front();
            boardsq.pop();

            boardq.push(p);
            s =  p.getVip() ? "(VIP)": "";
            cout << "passenger " << p.getId() << s << " has started waiting to be boarded at time " << gtime << endl;
            pthread_mutex_unlock(&timemtx);
            pthread_mutex_unlock(&boardmtx);

            sem_post(&boardsem);
            


            // not sure if the time is constant increasing or max amount of time

        }

       



    }


}





int main(void)
{	


    srand(time(0));
    // show_passenger();


    ifstream inputfile;
    fstream outputfile;
    
    inputfile.open("input.txt", ios::in);
    outputfile.open("output.txt", ios::out);

    if(!inputfile){
        cout << "no input file" << endl;
        return 0;
    }

    // freopen("output.txt", "w", stdout);

    
    inputfile >> M;
    inputfile >> N;
    inputfile >> P;
    inputfile >> w;
    inputfile >> x;
    inputfile >> y;
    inputfile >> z;


    cout << M << " " << N << " " << P << " " << w << " " << x << " " << y << " " << z << endl;


    beltmtx = new pthread_mutex_t[N];
    beltsem = new sem_t[P];
    beltq = new queue<passenger>[N];
    pthread_t kioskthreads[M], belthread[N], pasthread, vipleftthread, viprightthread, boardthread, specialkioskthread, timethread;
    pthread_mutex_init(&kioskmtx, NULL);
    pthread_mutex_init(&speckioskmtx, NULL);
    pthread_mutex_init(&vipleftmtx, NULL);
    pthread_mutex_init(&viprightmtx, NULL);
    pthread_mutex_init(&priorvipmtx, NULL);
    pthread_mutex_init(&boardmtx, NULL);
    pthread_mutex_init(&timemtx, NULL);
    pthread_mutex_init(&globalboolmtx, NULL);
    sem_init(&kiosksem, 0, 0);
    sem_init(&vipleftsem, 0, 0);
    sem_init(&viprightsem, 0, 0);
    sem_init(&specialkiosksem, 0, 0);
    sem_init(&boardsem, 0, 0);
    for(int i = 0; i < N; i++){

        pthread_mutex_init(&beltmtx[i], NULL);
        // no of mtx

    }

    for(int i = 0; i < P; i++){

        sem_init(&beltsem[i], 0, 0);
        // no of passenger in belt
    }

	int rc;
	for(int t = 0; t < M ; ){
        

        // rc = pthread_create(&threads[t], NULL, checkinKiosk, (void*) &temp);
        rc = pthread_create(&kioskthreads[t], NULL, checkinKiosk, (void*) (intptr_t) t);
        cout << "In main: creating different kiosk thread : " <<  t << endl;

        if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }

        t++;
        
    }


    for(int t = 0; t < N; ){
        

        // rc = pthread_create(&threads[t], NULL, checkinKiosk, (void*) &temp);
        // no of belts here
        rc = pthread_create(&belthread[t], NULL, securityCheck , (void*) (intptr_t) t);
        cout << "In main: creating different security thread : " <<  t << endl;

        if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }

        t++;
    }


    pthread_create(&vipleftthread, NULL, leftright, NULL);
    cout << "In main: creating vipleftq thread " << endl;
    pthread_create(&viprightthread, NULL, rightleft, NULL);
    cout << "In main: creating viprightq thread " << endl;

    pthread_create(&specialkioskthread, NULL, returnback, NULL);
    cout << "In main: creating speicalkiosk thread " << endl;
    pthread_create(&boardthread, NULL, boarding, NULL);
    cout << "In main: creating boarding thread " << endl;

    pthread_create(&timethread, NULL, timecount, NULL);

    rc = pthread_create(&pasthread, NULL, create_passenger, NULL);
    cout << "In main: creating different passenger thread " << endl;

    if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
    }

	while(1);
	return 0;
}
