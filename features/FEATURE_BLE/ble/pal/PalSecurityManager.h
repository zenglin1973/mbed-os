/* mbed Microcontroller Library
 * Copyright (c) 2017-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_OS_FEATURES_FEATURE_BLE_BLE_PAL_PALSM_H_
#define MBED_OS_FEATURES_FEATURE_BLE_BLE_PAL_PALSM_H_

#include "platform/Callback.h"
#include "platform/NonCopyable.h"
#include "ble/BLETypes.h"
#include "ble/SafeEnum.h"
#include "ble/BLEProtocol.h"
#include "ble/SecurityManager.h"

namespace ble {
namespace pal {

using SecurityManager::SecurityIOCapabilities_t;
using SecurityManager::IO_CAPS_NONE;
using SecurityManager::SecurityCompletionStatus_t;
using SecurityManager::SecurityMode_t;
using SecurityManager::LinkSecurityStatus_t;
using SecurityManager::Keypress_t;

/* please use typedef for porting not the types directly */

typedef SecurityManager::Passkey_t passkey_t;
typedef SecurityManager::C192_t c192_t;
typedef SecurityManager::R192_t r192_t;
typedef SecurityManager::C256_t c256_t;
typedef SecurityManager::R256_t r256_t;
typedef BLEProtocol::AddressBytes_t address_t;

typedef uint8_t irk_t[16];
typedef uint8_t csrk_t[16];
typedef uint8_t ltk_t[16];
typedef uint8_t ediv_t[8];
typedef uint8_t rand_t[2];
typedef uint32_t passkey_num_t;

typedef uint8_t key_distribution_t;

enum KeyDistributionFlags_t {
    KEY_DISTRIBUTION_NONE       = 0x00,
    KEY_DISTRIBUTION_ENCRYPTION = 0x01,
    KEY_DISTRIBUTION_IDENTITY   = 0x02,
    KEY_DISTRIBUTION_SIGNING    = 0x04,
    KEY_DISTRIBUTION_LINK       = 0x08,
    KEY_DISTRIBUTION_ALL        = 0x0F
};

typedef uint8_t authentication_t;

enum AuthenticationFlags_t {
    AUTHENTICATION_BONDING                = 0x01,
    AUTHENTICATION_MITM                   = 0x04, /* 0x02 missing because bonding uses two bits */
    AUTHENTICATION_SECURE_CONNECTIONS     = 0x08,
    AUTHENTICATION_KEYPRESS_NOTIFICATION  = 0x10
};

struct bonded_list_entry_t {
    address_t peer_address;
    ediv_t ediv;
    rand_t rand;
    ltk_t ltk;
    csrk_t csrk;
};

struct resolving_list_entry_t {
    address_t peer_address;
    irk_t peer_irk;
    irk_t local_irk;
};

/** Representation of a resolving list. */
struct resolving_list_t {
    resolving_list_entry_t *entries; /**< pointer to array storing the entries */
    uint8_t size; /**< actual number of entries */
    uint8_t capacity;  /**< number of entries that can be stored */
};

/** Representation of a bonded list. */
struct bonded_list_t {
    bonded_list_entry_t *entries; /**< pointer to array storing the entries */
    uint8_t size; /**< actual number of entries */
    uint8_t capacity;  /**< number of entries that can be stored */
};

/**
 * Handle events generated by ble::pal::SecurityManager
 */
class SecurityManagerEventHandler {
public:
    virtual void security_setup_initiated(
        connection_handle_t handle,
        bool allow_bonding,
        bool require_mitm,
        SecurityIOCapabilities_t iocaps
    ) = 0;

    virtual void security_setup_completed(
        connection_handle_t handle,
        SecurityManager::SecurityCompletionStatus_t status
    ) = 0;

    virtual void link_secured(
        connection_handle_t handle, SecurityManager::SecurityMode_t security_mode
    ) = 0;

    virtual void security_context_stored(connection_handle_t handle) = 0;

    virtual void passkey_display(connection_handle_t handle, const passkey_t passkey) = 0;

    virtual void valid_mic_timeout(connection_handle_t handle) = 0;

    virtual void link_key_failure(connection_handle_t handle) = 0;

    virtual void keypress_notification(connection_handle_t handle, SecurityManager::Keypress_t keypress) = 0;

    virtual void legacy_pariring_oob_request(connection_handle_t handle) = 0;

    virtual void oob_request(connection_handle_t handle) = 0;

    virtual void pin_request(connection_handle_t handle) = 0;

    virtual void passkey_request(connection_handle_t handle) = 0;

    virtual void confirmation_request(connection_handle_t handle) = 0;

    virtual void accept_pairing_request(
        connection_handle_t handle,
        SecurityIOCapabilities_t iocaps,
        bool use_oob,
        authentication_t authentication,
        uint8_t max_key_size,
        key_distribution_t initiator_dist,
        key_distribution_t responder_dist
    ) = 0;

    virtual void keys_exchanged(
        connection_handle_t handle,
        address_t &peer_address,
        ediv_t &ediv,
        rand_t &rand,
        ltk_t &ltk,
        csrk_t &csrk
    ) = 0;

    virtual void ltk_request(
        connection_handle_t handle,
        ediv_t &ediv,
        rand_t &rand
    ) = 0;
};

/**
 * Adaptation layer of the Security Manager.
 */
class SecurityManager : private mbed::NonCopyable<SecurityManager> {
public:
    SecurityManager() : _pal_event_handler(NULL) { };

    virtual ~SecurityManager() { };

    virtual ble_error_t initialize() = 0;

    virtual ble_error_t terminate() = 0;

    virtual ble_error_t reset()  = 0;

    /* persistence */

    virtual ble_error_t get_bonded_list(bonded_list_t &list) = 0;

    virtual ble_error_t add_bonded_list_entry(bonded_list_entry_t &entry) = 0;

    virtual ble_error_t remove_bonded_list_entry(bonded_list_entry_t &entry) = 0;

    virtual ble_error_t clear_bonded_list() = 0;

    virtual ble_error_t get_resolving_list(resolving_list_t &list) = 0;

    virtual ble_error_t add_resolving_list_entry(resolving_list_entry_t &entry) = 0;

    virtual ble_error_t remove_resolving_list_entry(resolving_list_entry_t &entry) = 0;

    virtual ble_error_t clear_resolving_list() = 0;

    /* feature support */

    virtual ble_error_t set_secure_connections_support(
        bool enabled, bool secure_connections_only = false
    ) = 0;

    virtual ble_error_t get_secure_connections_support(
        bool &enabled, bool &secure_connections_only
    ) = 0;

    /* security settings */

    virtual ble_error_t set_pin_code(
        uint8_t pin_length, uint8_t *pin_code, bool static_pin = false
    ) = 0;

    virtual ble_error_t set_passkey(passkey_num_t passkey) = 0;

    virtual ble_error_t set_authentication_timeout(
        connection_handle_t, uint16_t timeout_in_10ms
    ) = 0;

    virtual ble_error_t get_authentication_timeout(
        connection_handle_t, uint16_t &timeout_in_10ms
    ) = 0;

    /* encryption */

    virtual ble_error_t enable_encryption(connection_handle_t handle) = 0;

    virtual ble_error_t disable_encryption(connection_handle_t handle) = 0;

    virtual ble_error_t get_encryption_status(
        connection_handle_t handle, LinkSecurityStatus_t &status
    ) = 0;

    virtual ble_error_t get_encryption_key_size(
        connection_handle_t, uint8_t &bitsize
    ) = 0;

    virtual ble_error_t refresh_encryption_key(connection_handle_t handle) = 0;

    /* privacy */

    virtual ble_error_t set_private_address_timeout(uint16_t timeout_in_seconds) = 0;

    /* keys */

    virtual ble_error_t set_ltk(connection_handle_t handle, ltk_t ltk) = 0;

    virtual ble_error_t set_irk(irk_t irk) = 0;

    virtual ble_error_t set_csrk(csrk_t csrk) = 0;

    virtual ble_error_t generate_irk() = 0;

    virtual ble_error_t generate_csrk() = 0;

    /* authentication */

    virtual ble_error_t request_pairing(
        connection_handle_t handle,
        SecurityIOCapabilities_t iocaps,
        bool use_oob,
        authentication_t authentication,
        uint8_t max_key_size,
        key_distribution_t initiator_dist,
        key_distribution_t responder_dist
    ) = 0;

    virtual ble_error_t accept_pairing(
        connection_handle_t handle,
        SecurityIOCapabilities_t iocaps,
        bool use_oob,
        authentication_t authentication,
        uint8_t max_key_size,
        key_distribution_t initiator_dist,
        key_distribution_t responder_dist
    ) = 0;

    virtual ble_error_t reject_pairing(connection_handle_t handle) = 0;

    virtual ble_error_t cancel_pairing(connection_handle_t handle) = 0;

    virtual ble_error_t set_pairing_request_authorisation(
        bool authorisation_required = true
    ) = 0;

    virtual ble_error_t request_authentication(connection_handle_t handle) = 0;

    /* MITM */

    virtual ble_error_t confirmation_entered(
        connection_handle_t handle, bool confirmation
    ) = 0;

    virtual ble_error_t passkey_entered(
        connection_handle_t handle, passkey_t passkey
    ) = 0;

    virtual ble_error_t send_keypress_notification(
        connection_handle_t handle, Keypress_t keypress
    ) = 0;

    virtual ble_error_t set_oob(
        connection_handle_t handle, c192_t& c192, r192_t& r192
    ) = 0;

    virtual ble_error_t set_extended_oob(
        connection_handle_t handle,
        c192_t& c192,
        r192_t& r192,
        c256_t& c256,
        r256_t& r256
    ) = 0;

    virtual ble_error_t get_local_oob_data(
        connection_handle_t handle, c192_t& c192, r192_t& r192
    ) = 0;

    virtual ble_error_t get_local_extended_oob_data(
        connection_handle_t handle,
        c192_t& c192, r192_t& r192, c256_t& c256, r256_t& r256
    ) = 0;


    /* Entry points for the underlying stack to report events back to the user. */
public:
    void set_event_handler(SecurityManagerEventHandler *event_handler) {
        _pal_event_handler = event_handler;
    }


protected:
    SecurityManagerEventHandler* get_event_handler() {
        return _pal_event_handler;
    }

private:
    SecurityManagerEventHandler *_pal_event_handler;

};

} /* namespace pal */
} /* namespace ble */

#endif /* MBED_OS_FEATURES_FEATURE_BLE_BLE_PAL_PALSM_H_ */
