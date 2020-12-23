cat /etc/nginx/sites-available/sfu 
server {

  listen 80 default_server;

  server_name _;

  return 301 https://$host$request_uri;


  #listen 80;

  #server_name sfu.scopear.com;

  #location / {

      #proxy_set_header X-Real-IP $remote_addr;
      #proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
      #proxy_set_header X-Forwarded-Proto $scheme;
      #proxy_set_header Host $http_host;
      #proxy_redirect off;
      
      # enable WebSockets
      #proxy_http_version 1.1;
      #proxy_set_header Upgrade $http_upgrade;
      #proxy_set_header Connection "upgrade";      

      #proxy_pass https://127.0.0.1:8080;
  #}

}


server {
  #listen 80;
  listen 443 ssl;

  server_name sfu.scopear.com;

  ssl on;
  ssl_certificate     /etc/letsencrypt/live/sfu.scopear.com/fullchain.pem;
  ssl_certificate_key /etc/letsencrypt/live/sfu.scopear.com/privkey.pem;

  location / {
    
      proxy_set_header X-Real-IP $remote_addr;
      proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
      proxy_set_header X-Forwarded-Proto $scheme;
      proxy_set_header Host $http_host;
      #proxy_redirect off;

      # enable WebSockets
      proxy_http_version 1.1;
      proxy_set_header Upgrade $http_upgrade;
      proxy_set_header Connection "upgrade";

      proxy_pass https://127.0.0.1:8080;
  }

}


/etc/letsencrypt/live/sfu.scopear.com# ls
README  cert.pem  chain.pem  fullchain.pem  privkey.pem

