use std::{
    io::{Read, Write},
    net::{TcpListener, TcpStream},
    thread,
};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let listener = TcpListener::bind("127.0.0.1:9090").unwrap();

    for stream in listener.incoming() {
        if let Err(err) = thread::spawn(move || {
            || -> Result<(), Box<dyn std::error::Error>> {
                let mut stream = stream?;

                let mut buf = String::new();
                stream.read_to_string(&mut buf)?;

                match buf.as_str() {
                    "ping" => {
                        let mut ip = stream.peer_addr()?.clone();
                        ip.set_port(9091);
                        if let Ok(mut socket) = TcpStream::connect(ip) {
                            write!(socket, "Pong!\n")?;
                            println!("ponged");
                        };
                    }
                    _ => {
                        println!("unknown command {}", buf.as_str());
                    }
                }
                Ok(())
            }()
            .unwrap();
        })
        .join()
        {
            println!("{:?}", err);
        }
    }

    Ok(())
}
