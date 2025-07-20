Name:       runWebrtc
Version:    1.0
Release:    1%{?dist}
Summary:    runWebrtc
License:    GPLv3+
BuildArch:  x86_64

%description
Video Edge Webrtc!

#Requires: webrtcdep

%prep

%build

%install
mkdir -p %{buildroot}/%{_des_webrtc}
mkdir -p %{buildroot}/etc/systemd/system
mkdir -p %{buildroot}/var/tmp/key
mkdir -p %{buildroot}/%{_des_signalling}
mkdir -p %{buildroot}/%{_des_signalling}/js
mkdir -p %{buildroot}/%{_des_signalling}/css
mkdir -p %{buildroot}/var/log/webrtcserver
install -m 0755 %{_src_webrtc}/%{name} %{buildroot}/%{_des_webrtc}/%{name}
cp %{_src_webrtc}/config.js  %{buildroot}/%{_des_webrtc}/
cp %{_src_webrtc}/webrtc.service %{_src_webrtc}/signalling.service %{buildroot}/etc/systemd/system
cp %{_src_webrtc}/key/*  %{buildroot}/var/tmp/key
cp %{_src_signalling}/config.js    %{buildroot}/%{_des_signalling}/config.js
cp %{_src_signalling}/css/main.css  %{buildroot}/%{_des_signalling}/css/main.css
cp %{_src_signalling}/favicon.ico   %{buildroot}/%{_des_signalling}/favicon.ico
cp %{_src_signalling}/index.html    %{buildroot}/%{_des_signalling}/index.html
cp %{_src_signalling}/index.js      %{buildroot}/%{_des_signalling}/index.js
cp %{_src_signalling}/js/adapter-latest.js %{buildroot}/%{_des_signalling}/js/adapter-latest.js
cp %{_src_signalling}/js/app.js   %{buildroot}/%{_des_signalling}/js/app.js
cp %{_src_signalling}/js/main.js  %{buildroot}/%{_des_signalling}/js/main.js
cp %{_src_signalling}/js/sdp.js   %{buildroot}/%{_des_signalling}/js/sdp.js
cp %{_src_signalling}/js/webRtcPlayer.js %{buildroot}/%{_des_signalling}/js/webRtcPlayer.js
cp %{_src_signalling}/package.json  %{buildroot}/%{_des_signalling}/package.json
cp %{_src_signalling}/README.md  %{buildroot}/var/log/webrtcserver/README.md

#%clean
#rm -rf %{buildroot}

%files
%{_des_webrtc}/%{name}
%{_des_webrtc}/config.js
/etc/systemd/system/webrtc.service
/etc/systemd/system/signalling.service
/var/tmp/key/ssl-cert-snakeoil.key
/var/tmp/key/ssl-cert-snakeoil.pem 
/var/tmp/key/certificate.crt
/var/tmp/key/private_key.pem
%{_des_signalling}/config.js
%{_des_signalling}/css/main.css
%{_des_signalling}/favicon.ico
%{_des_signalling}/index.html
%{_des_signalling}/index.js
%{_des_signalling}/js/adapter-latest.js
%{_des_signalling}/js/app.js
%{_des_signalling}/js/main.js
%{_des_signalling}/js/sdp.js
%{_des_signalling}/js/webRtcPlayer.js
%{_des_signalling}/package.json
/var/log/webrtcserver/README.md




%post
npm install --prefix %{_des_signalling}  --production
%systemd_post signalling.service
%systemd_post webrtc.service

%preun
%systemd_preun signalling.service
%systemd_preun webrtc.service


%postun
%systemd_postun_with_restart signalling.service
%systemd_postun_with_restart webrtc.service



%changelog
