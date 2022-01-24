#ifndef _SecurityToken_hxx_
#define _SecurityToken_hxx_

/*
 * Copyright (c) 2011-2013 byArvind Umrao.
 * 
 */


#include "Digest.h"

class SecurityToken
{
	public:

		static string createServerToken(string& value);
		static bool isValidServerToken(string& token, string& value, uint32_t timeDiff = 300);
};

class MS_SecurityToken 
{
	public:
		MS_SecurityToken(const string& token) 
		{
			clear();
			m_token.assign(token);
		}
		
		MS_SecurityToken()
		{
			clear();	
		}
		
		~MS_SecurityToken()  
		{
			clear();
		}
		
		void clear()  {
			m_token.clear();
			m_cameraUID.clear();
			m_permissions.clear();
			m_h2.clear();
			m_qosl = 0;
			m_qosa = 0;
			m_ptzPriority = 0;
			m_expirationTime = 0;
		}

		enum { OK = 200, UNAUTHORIZED = 401, AUTHORIZATION_EXPIRED = 402, FORBIDDEN = 403 };
			
		const string& getToken() const {
			return m_token;
		}
	
		uint8_t getQosl() const {
			return m_qosl;
		}
		
		uint8_t getQosa() const {
			return m_qosa;
		}

		uint32_t getExpirationTime() const {
			return m_expirationTime;
		}

		uint8_t getPTZPriority() const {
			return m_ptzPriority;
		}

		const string getPermissions() const {
			return m_permissions;
		}

		const string& getCameraUID() const {
			return m_cameraUID;
		}

		const string& getH2() const {
			return m_h2;
		}

		void init(const string& token) {
			m_token.assign(token);
		}

		bool validate(const string& key, 
					string& emsg, 
					 string& perm, 
					uint32_t& statusCode,
					bool rtsp = false);

//		const bool isTokenFor(const string& uid) const {
//			 return uid.starts_with(m_cameraUID);
//		}

		static const char SECURITY_TOKEN_SEPARATOR = '^';

		static string createSecurityToken(string& uid,
                                          string& permissions,
                                          const string& key,
                                          uint32_t expirationTime,
                                          uint8_t ptz=0,
                                          uint8_t qosl=0,
                                          uint8_t qosa=0);

	private:
		bool parse();
		static string calculateH2(string& uid,
    				string& permissions,
					uint8_t ptz,
					uint8_t qosl,
					uint8_t qosa,
    				const string& key,
    				uint32_t expirationTime);

		static const int SECURITY_TOKEN_ITEMS = 7;
		enum { CAMUID = 0, PERM, PTZPRI, QOSL, QOSA, EXPIRATION, H2};

		string m_token;
		string m_cameraUID;
		string m_permissions;
		uint8_t m_qosl;
		uint8_t m_qosa;
		uint8_t m_ptzPriority;
		uint32_t m_expirationTime;
		string m_h2;
};


#endif
