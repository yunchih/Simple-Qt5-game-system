#include "game_system.hpp"

Connection connection;
GuessNumServer* guess_num;
BlackJackServer* blackjack;

void initialize_game(Connection&& conn){
    connection = conn; 
    guess_num = new GuessNumServer(CONFIG::ROOM_NUMBER, CONFIG::PLAYER_NUMBER);
    blackjack = new BlackJackServer(CONFIG::ROOM_NUMBER, CONFIG::PLAYER_NUMBER);
}

void start_game(){

    while(1){
        int req = connection.accept();        
        std::cerr << "New connection: fd = " << req << std::endl;

        if( req < 0 )
            std::cerr << "Fail accepting incomming connection\n";
        else{
            std::thread spawn( process, req );
            spawn.detach();
        }
    }
}

static void process(int req_socket){
    
    Connection::set_timeout(req_socket);

    System::User user;

    int timeout_count = 0;

    int cnt = 0;

    bool keep_running = true;

    while(keep_running){

        Model::Request request;
        if( !Connection::receive_message(req_socket, request) ){

            /* Handle Timeout */
            std::cerr << "Error or timeout\n"; 

            if( ++timeout_count > TIMEOUT_LIMIT ){
                if( user.has_game_type() /* If the user is in some game */){ 
                    /* purge from room  */
                    switch( user.game_type() ){
                        case System::GUESSNUM:
                            guess_num->kick_off( user );
                            break;

                        case System::JACK:
                            blackjack->kick_off( user );
                            break;
                    }
                }

                // Purge from game
                keep_running = false;
                continue;
            }
        }
        else{

            std::cerr << "Getting a new request: " << cnt++ << std::endl;
            timeout_count = 0;
        }
        
        bool status = true;
        Model::Reply response;

        if( request.has_blackjack() ){
            status = blackjack->handle_req( user, request.blackjack(), response.mutable_blackjack() );
        }
        else if( request.has_guess_num() ){
            status = guess_num->handle_req( user, request.guess_num(), response.mutable_guess_num() );
        }
        else if( request.has_system() ){
            const System::Request& req = request.system();
            auto response_user_info = response.mutable_system()->mutable_user();
            auto op = req.operation();
            switch( op ){ 

                case System::Request_Operation_KeepAlive: 
                    break;

                case System::Request_Operation_NewUser: 
                    if( req.has_name() )
                        user.set_name( req.name() );
                    else
                        user.set_name( std::string("Anomynous") );

                    initialize_user( user );
                    *response_user_info = user;
                    break;

                case  System::Request_Operation_QuitGame:
                    std::cerr << "User leaving the game\n";
                    keep_running = false;
                    break;

                default:
                    if( req.has_game_type() )
                        user.set_game_type( req.game_type() );

                    if( user.has_game_type() ){
                        switch( user.game_type() ){
                            case System::GUESSNUM:
                                status = guess_num->do_operation( op, user, response_user_info  );
                                break;

                            case System::JACK:
                                status = blackjack->do_operation( op, user, response_user_info );
                                break;
                        }
                    }
            }
        }

        response.set_status(status);

        if( !Connection::send_message(req_socket, response) ){
            /* TODO */
            /* Handle Timeout */

            std::cerr << "Sending timeout\n";
        }
    }

    /* Do some cleanup here */

    std::cerr << "Connection fd = " << req_socket << " closed\n";

    ::close( req_socket );

}

static void initialize_user( System::User& user ){
    user.set_money( CONFIG::INITIAL_MONEY );
    user.set_game_type( System::NONE );
}
