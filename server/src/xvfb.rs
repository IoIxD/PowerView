use std::result::Result;
use std::{
    process::{Child, Command, Stdio},
    time::SystemTime,
};
use xcb::*;
pub struct XVFB {
    pub xchild: Child,
    pub pchild: Child,
    pub conn: Connection,
    pub screen_num: i32,
}

impl XVFB {
    pub fn new(cmd: impl Into<String>) -> Result<Self, Box<dyn std::error::Error>> {
        // Start Xfvb at the next avaliable slot.
        let mut i = 0;
        println!("Starting Xvfb on the next avaliable slot");
        loop {
            let mut command = Command::new("Xvfb");
            let disp = format!(":{}", i);
            let command = command.args(vec![disp.as_str(), "-screen", "0", "640x480x24"]);

            let mut ch = command
                .stdout(Stdio::piped())
                .stderr(Stdio::piped())
                .spawn()?;

            let time = SystemTime::now();

            while time.elapsed()?.as_millis() <= 250 {}

            if let None = ch.try_wait()? {
                println!("Xvfb started on :{}, pid {}", i, ch.id());

                let (conn, screen_num) = xcb::Connection::connect(Some(&disp))?;

                let cmd = cmd.into();
                let pch = Command::new(cmd.clone())
                    .env("DISPLAY", disp.clone())
                    .spawn()?;

                return Ok(Self {
                    xchild: ch,
                    pchild: pch,
                    conn,
                    screen_num,
                });
            }
            i += 1;
        }
    }

    pub fn data(&self) -> Result<Vec<u8>, Box<dyn std::error::Error>> {
        // Fetch the `x::Setup` and get the main `x::Screen` object.
        let conn = &self.conn;
        let setup = conn.get_setup();
        let screen = setup.roots().nth(self.screen_num as usize).unwrap();

        let width = screen.width_in_pixels();
        let height = screen.height_in_pixels();

        let cookie = conn.send_request(&x::GetImage {
            format: x::ImageFormat::ZPixmap,
            drawable: x::Drawable::Window(screen.root()),
            x: 0,
            y: 0,
            width,
            height,
            plane_mask: u32::MAX,
        });
        let reply = conn.wait_for_reply(cookie).unwrap();
        let src = reply.data();
        let mut data = vec![0; width as usize * height as usize * 3];
        for (src, dest) in src.chunks(4).zip(data.chunks_mut(3)) {
            // Captured image stores orders pixels as BGR, so we need to reorder them.
            dest[0] = src[2];
            dest[1] = src[1];
            dest[2] = src[0];
        }

        Ok(data)
    }
}

impl Drop for XVFB {
    fn drop(&mut self) {
        unsafe {
            libc::kill(self.xchild.id() as i32, libc::SIGTERM);
        };
        unsafe {
            libc::kill(self.pchild.id() as i32, libc::SIGTERM);
        };
    }
}
