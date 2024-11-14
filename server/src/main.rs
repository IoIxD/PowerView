use std::{
    io::{Read, Write},
    net::{SocketAddr, TcpListener, TcpStream, UdpSocket},
    sync::Arc,
    thread,
};

use hashbrown::HashMap;
use parking_lot::Mutex;
use xvfb::XVFB;

mod xvfb;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let listener = TcpListener::bind("127.0.0.1:9090").unwrap();

    let progs = Arc::new(Mutex::new(HashMap::new()));

    let mut addrs = vec![];
    for i in 9091..u16::MAX {
        addrs.push(SocketAddr::from(([127, 0, 0, 1], i)));
    }
    let addrs = Arc::new(addrs);
    for stream in listener.incoming() {
        let p = progs.clone();
        let addrs = addrs.clone();
        if let Err(err) = thread::spawn(move || {
            || -> Result<(), Box<dyn std::error::Error>> {
                let mut stream = stream?;

                let mut buf = String::new();
                stream.read_to_string(&mut buf)?;

                let parts = buf
                    .split(";")
                    .filter(|f| !f.is_empty())
                    .collect::<Vec<&str>>();

                if let Some(cmd) = parts.get(0) {
                    // TODO: Dynamically assigned port
                    match *cmd {
                        // LAUNCH;prog_name
                        "LAUNCH" => {
                            if let Some(prog) = parts.get(1) {
                                p.lock().insert(prog.to_string(), XVFB::new(*prog)?);
                            }
                        }
                        // DATA;prog_name
                        "DATA" => {
                            if let Some(prog) = parts.get(1) {
                                let mut p = p.lock();
                                if let Some(win) = p.get_mut(*prog) {
                                    if let Err(err) =
                                        || -> Result<(), Box<dyn std::error::Error>> {
                                            let mut ip = stream.peer_addr()?.clone();
                                            ip.set_port(9091);
                                            if let Ok(socket) = UdpSocket::bind(&addrs[..]) {
                                                socket.connect(ip)?;

                                                // COMMAND
                                                socket.send("0:DATA".as_bytes())?;

                                                // SIZE
                                                let data = win.data()?;
                                                socket
                                                    .send(format!("1:{}", data.len()).as_bytes())?;

                                                // DATA
                                                for f in data.chunks(1024) {
                                                    let mut hdr = "2:".as_bytes().to_vec();
                                                    hdr.append(&mut f.to_vec());

                                                    socket.send(&hdr)?;
                                                }
                                            };
                                            Ok(())
                                        }()
                                    {
                                        println!(
                                            "got error {}, removing program.",
                                            err.to_string()
                                        );
                                        p.remove(*prog);
                                    }
                                }
                            }
                        }
                        _ => {
                            println!("unknown command {}", buf.as_str());
                        }
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
