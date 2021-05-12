(*
 * Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *)

open preamble basis @DEPENDENCY_PATH@Theory;

val _ = new_theory "virtqueue";

val _ = translation_extends "@DEPENDENCY_PATH@";

val _ = ml_prog_update (open_module "VirtQueue");

val virtqueue_datatype_def = (append_prog o process_topdecs) `
    datatype virtqueue = VirtQueueDrv word8array | VirtQueueDev word8array
`;

(* Datatype to indicate whether a call succeeded with a result 'a, or failed with an error 'e *)
val result_datatype_def = (append_prog o process_topdecs) `
    datatype ('a, 'e) result = Ok 'a | Err 'e
`;

(* If a result is Ok, fetch its value, else halt the program by calling fail *)
val unwrap_def = (append_prog o process_topdecs) `
    fun unwrap res = case res of
        Ok v => v
    |   Err e => Utils.fail "unhandled virtqueue error"
`;

val get_result_def = (append_prog o process_topdecs) `
    fun get_result buf f =
        (* check if first byte of FFI array is FFI_SUCCESS (0) *)
        if Utils.bytes_to_int buf 0 1 = 0 then
            Ok (f buf)
        else
            Err ()
`;

val _ = (append_prog o process_topdecs) `
    fun virtqueue_ptr vq = case vq of
        VirtQueueDrv ptr => ptr
    |   VirtQueueDev ptr => ptr
`;

val device_init_def = (append_prog o process_topdecs) `
    fun device_init virtqueue virtqueue_id =
        let
            val buf = Word8Array.array 9 (Word8.fromInt 0);
            val _ = Word8Array.copy (Utils.int_to_bytes virtqueue_id 4) 0 4 buf 1;
            val _ = #(virtqueue_device_init) "" buf;
        in
            get_result buf (fn buf => Word8Array.copy buf 1 8 (virtqueue_ptr virtqueue) 0)
        end
`;

val driver_init_def = (append_prog o process_topdecs) `
    fun driver_init virtqueue virtqueue_id =
        let
            val buf = Word8Array.array 9 (Word8.fromInt 0);
            val _ = Word8Array.copy (Utils.int_to_bytes virtqueue_id 4) 0 4 buf 1;
            val _ = #(virtqueue_driver_init) "" buf;
        in
            get_result buf (fn buf => Word8Array.copy buf 1 8 (virtqueue_ptr virtqueue) 0)
        end
`;

val device_poll_def = (append_prog o process_topdecs) `
    fun device_poll virtqueue = let
        val buf = Word8Array.array 9 (Word8.fromInt 0);
        val _ = Word8Array.copy virtqueue 0 8 buf 1;
        val _ = #(virtqueue_device_poll) "" buf;
        in get_result buf (fn buf => Utils.bytes_to_int buf 1 4) end
`;

val _ = (append_prog o process_topdecs) `
    fun driver_poll virtqueue = let
        val buf = Word8Array.array 9 (Word8.fromInt 0);
        val _ = Word8Array.copy virtqueue 0 8 buf 1;
        val _ = #(virtqueue_driver_poll) "" buf;
        in get_result buf (fn buf => Utils.bytes_to_int buf 1 4) end
`;

val device_recv_def = (append_prog o process_topdecs) `
    fun device_recv virtqueue =
        let
            (* TODO: set size to max virtqueue buf size (statically or by asking VQ lib) *)
            val buf = Word8Array.array (1 + 8 + 4096) (Word8.fromInt 0);
            val _ = Word8Array.copy (virtqueue_ptr virtqueue) 0 8 buf 1;
            val _ = #(virtqueue_device_recv) "" buf;
        in
            get_result buf (fn buf => let
                val result_size = Utils.bytes_to_int buf 1 8;
                val result = Word8Array.array result_size (Word8.fromInt 0);
                val _ = Word8Array.copy buf (1 + 8) result_size result 0;
                in result end)
        end
`;

val driver_recv_def = (append_prog o process_topdecs) `
    fun driver_recv virtqueue = let
        val buf = Word8Array.array 9 (Word8.fromInt 0);
        val _ = Word8Array.copy (virtqueue_ptr virtqueue) 0 8 buf 1;
        val _ = #(virtqueue_driver_recv) "" buf;
        in get_result buf (fn buf => ()) end
`;

val driver_send_def = (append_prog o process_topdecs) `
    fun driver_send virtqueue message = let
        val message_len = Word8Array.length message;
        val buf_size = 1 + 8 + 4 + message_len;
        val buf = Word8Array.array buf_size (Word8.fromInt 0);
        val _ = Word8Array.copy (virtqueue_ptr virtqueue) 0 8 buf 1;
        val _ = Word8Array.copy (Utils.int_to_bytes message_len 4) 0 4 buf (1 + 8);
        val _ = Word8Array.copy message 0 message_len buf (1 + 8 + 4);
        val _ = #(virtqueue_driver_send) "" buf;
        in get_result buf (fn buf => ()) end
`;

val device_signal_def = (append_prog o process_topdecs) `
    fun device_signal virtqueue = let
        val buf = Word8Array.array 9 (Word8.fromInt 0);
        val _ = Word8Array.copy (virtqueue_ptr virtqueue) 0 8 buf 1;
        val _ = #(virtqueue_device_signal) "" buf;
        in get_result buf (fn buf => ()) end
`;

val _ = (append_prog o process_topdecs) `
    fun driver_signal virtqueue = let
        val buf = Word8Array.array 9 (Word8.fromInt 0);
        val _ = Word8Array.copy (virtqueue_ptr virtqueue) 0 8 buf 1;
        val _ = #(virtqueue_driver_signal) "" buf;
        in get_result buf (fn buf => ()) end
`;

val _ = (append_prog o process_topdecs) `
    fun poll vq = case vq of
        VirtQueueDev ptr => device_poll ptr
    |   VirtQueueDrv ptr => driver_poll ptr
`;

val select_def = (append_prog o process_topdecs) `
    fun select vqs = List.find (fn vq => poll vq = Ok 1) vqs
`;

val _ = ml_prog_update (close_module NONE);

val _ = export_theory ();
