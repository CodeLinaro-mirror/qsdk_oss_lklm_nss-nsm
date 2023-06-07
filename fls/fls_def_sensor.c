/*
 **************************************************************************
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include "fls_conn.h"
#include "fls_chardev.h"
#include "fls_def_sensor.h"
#include "fls_debug.h"

#define FLS_DEF_SENSOR_DELAY_DEF 0
#define FLS_DEF_SENSOR_SAMPLE_LEN_DEF 0
#define FLS_DEF_SENSOR_MAX_EVENTS_DEF -1
#define FLS_DEF_SENSOR_DYNAMIC_SAMPLES_DEF true

uint32_t fls_def_sensor_delay;
uint32_t fls_def_sensor_sample_length;
int32_t fls_def_sensor_max_events;
uint32_t fls_def_sensor_sample_count;
uint32_t fls_def_sensor_bytes;
uint32_t fls_def_sensor_ipat;
uint32_t fls_def_sensor_burst;
uint32_t fls_def_sensor_burst_threshold;
uint32_t fls_def_sensor_burst_short_intvl;
uint32_t fls_def_sensor_burst_long_intvl;
bool fls_def_sensor_dynamic_samples;
static struct fls_event event;
uint32_t fls_def_sensor_pkts_hwm;
uint32_t fls_def_sensor_bytes_hwm;

static void fls_def_sensor_event_create(struct fls_conn *conn, ktime_t time)
{
	uint32_t i;
	struct fls_conn *orig;
	struct fls_conn *reverse;

	if (!conn->reverse) {
		FLS_WARN("%p cannot create event for unidirectional flow.", conn);
		return;
	}

	if (conn->dir == FLS_CONN_DIRECTION_ORIG) {
		orig = conn;
		reverse = conn->reverse;
	} else {
		orig = conn->reverse;
		reverse = conn;
	}

	event.event_type = FLS_CHARDEV_EVENT_TYPE_DEF;
	event.dir = 0xEB;
	event.ip_version = conn->ip_version;
	event.protocol = conn->protocol;

	event.orig_src_port = orig->src_port;
	event.orig_dest_port = orig->dest_port;
	event.orig_src_ip[0] = orig->src_ip[0];
	event.orig_src_ip[1] = orig->src_ip[1];
	event.orig_src_ip[2] = orig->src_ip[2];
	event.orig_src_ip[3] = orig->src_ip[3];
	event.orig_dest_ip[0] = orig->dest_ip[0];
	event.orig_dest_ip[1] = orig->dest_ip[1];
	event.orig_dest_ip[2] = orig->dest_ip[2];
	event.orig_dest_ip[3] = orig->dest_ip[3];

	event.ret_src_port = reverse->src_port;
	event.ret_dest_port = reverse->dest_port;
	event.ret_src_ip[0] = reverse->src_ip[0];
	event.ret_src_ip[1] = reverse->src_ip[1];
	event.ret_src_ip[2] = reverse->src_ip[2];
	event.ret_src_ip[3] = reverse->src_ip[3];
	event.ret_dest_ip[0] = reverse->dest_ip[0];
	event.ret_dest_ip[1] = reverse->dest_ip[1];
	event.ret_dest_ip[2] = reverse->dest_ip[2];
	event.ret_dest_ip[3] = reverse->dest_ip[3];

	event.timestamp = time;

	event.def_event.sample_count = fls_def_sensor_sample_count;
	event.def_event.sample_length_ms = fls_def_sensor_sample_length;

	for (i = 0; i < fls_def_sensor_sample_count; i++) {
		event.def_event.samples[i].orig_packets = orig->stats.isd.samples[i].packets;
		event.def_event.samples[i].orig_bytes = orig->stats.isd.samples[i].bytes;
		event.def_event.samples[i].orig_bytes_min = orig->stats.isd.samples[i].bytes_min;
		event.def_event.samples[i].orig_bytes_max = orig->stats.isd.samples[i].bytes_max;
		event.def_event.samples[i].orig_delta_sum = orig->stats.isd.samples[i].delta_sum;
		event.def_event.samples[i].orig_delta_min = orig->stats.isd.samples[i].delta_min;
		event.def_event.samples[i].orig_delta_max = orig->stats.isd.samples[i].delta_max;
		event.def_event.samples[i].orig_bursts = orig->stats.isd.samples[i].bursts;
		event.def_event.samples[i].orig_burst_sz_sum = orig->stats.isd.samples[i].burst_sz_sum;
		event.def_event.samples[i].orig_burst_sz_min = orig->stats.isd.samples[i].burst_sz_min;
		event.def_event.samples[i].orig_burst_sz_max = orig->stats.isd.samples[i].burst_sz_max;
		event.def_event.samples[i].orig_burst_dur_sum = orig->stats.isd.samples[i].burst_dur_sum;
		event.def_event.samples[i].orig_burst_dur_min = orig->stats.isd.samples[i].burst_dur_min;
		event.def_event.samples[i].orig_burst_dur_max = orig->stats.isd.samples[i].burst_dur_max;

		orig->stats.isd.samples[i].packets = 0;
		orig->stats.isd.samples[i].bytes = 0;
		orig->stats.isd.samples[i].bytes_min = 0;
		orig->stats.isd.samples[i].bytes_max = 0;
		orig->stats.isd.samples[i].delta_sum = 0;
		orig->stats.isd.samples[i].delta_min = 0;
		orig->stats.isd.samples[i].delta_max = 0;
		orig->stats.isd.samples[i].last_packet_time = 0;
		orig->stats.isd.samples[i].bursts = 0;
		orig->stats.isd.samples[i].burst_sz_sum = 0;
		orig->stats.isd.samples[i].burst_sz_min = 0;
		orig->stats.isd.samples[i].burst_sz_max = 0;
		orig->stats.isd.samples[i].burst_dur_sum = 0;
		orig->stats.isd.samples[i].burst_dur_min = 0;
		orig->stats.isd.samples[i].burst_dur_max = 0;

		event.def_event.samples[i].ret_packets = reverse->stats.isd.samples[i].packets;
		event.def_event.samples[i].ret_bytes = reverse->stats.isd.samples[i].bytes;
		event.def_event.samples[i].ret_bytes_min = reverse->stats.isd.samples[i].bytes_min;
		event.def_event.samples[i].ret_bytes_max = reverse->stats.isd.samples[i].bytes_max;
		event.def_event.samples[i].ret_delta_sum = reverse->stats.isd.samples[i].delta_sum;
		event.def_event.samples[i].ret_delta_min = reverse->stats.isd.samples[i].delta_min;
		event.def_event.samples[i].ret_delta_max = reverse->stats.isd.samples[i].delta_max;
		event.def_event.samples[i].ret_bursts = reverse->stats.isd.samples[i].bursts;
		event.def_event.samples[i].ret_burst_sz_sum = reverse->stats.isd.samples[i].burst_sz_sum;
		event.def_event.samples[i].ret_burst_sz_min = reverse->stats.isd.samples[i].burst_sz_min;
		event.def_event.samples[i].ret_burst_sz_max = reverse->stats.isd.samples[i].burst_sz_max;
		event.def_event.samples[i].ret_burst_dur_sum = reverse->stats.isd.samples[i].burst_dur_sum;
		event.def_event.samples[i].ret_burst_dur_min = reverse->stats.isd.samples[i].burst_dur_min;
		event.def_event.samples[i].ret_burst_dur_max = reverse->stats.isd.samples[i].burst_dur_max;


		reverse->stats.isd.samples[i].packets = 0;
		reverse->stats.isd.samples[i].bytes = 0;
		reverse->stats.isd.samples[i].bytes_min = 0;
		reverse->stats.isd.samples[i].bytes_max = 0;
		reverse->stats.isd.samples[i].delta_sum = 0;
		reverse->stats.isd.samples[i].delta_min = 0;
		reverse->stats.isd.samples[i].delta_max = 0;
		reverse->stats.isd.samples[i].last_packet_time = 0;
		reverse->stats.isd.samples[i].bursts = 0;
		reverse->stats.isd.samples[i].burst_sz_sum = 0;
		reverse->stats.isd.samples[i].burst_sz_min = 0;
		reverse->stats.isd.samples[i].burst_sz_max = 0;
		reverse->stats.isd.samples[i].burst_dur_sum = 0;
		reverse->stats.isd.samples[i].burst_dur_min = 0;
		reverse->stats.isd.samples[i].burst_dur_max = 0;

	}

	if (!fls_chardev_enqueue(&event)) {
		FLS_WARN("Event dropped!\n");
	}
}

static void fls_def_sensor_bytes_record(struct fls_def_sensor_sample *sample, uint32_t bytes)
{
	if (sample->packets == 0) {
		sample->bytes = bytes;
		sample->bytes_min = bytes;
		sample->bytes_max = bytes;
		return;
	}

	sample->bytes += bytes;
	if (bytes < sample->bytes_min) {
		sample->bytes_min = bytes;
	} else if (bytes > sample->bytes_max) {
		sample->bytes_max = bytes;
	}
}

static void fls_def_sensor_burst_open(struct fls_def_sensor_sample *sample, ktime_t now, uint32_t bytes) {
	sample->burst_data.start = now;
	sample->burst_data.last = now;
	sample->burst_data.sz = bytes;
	if (bytes > fls_def_sensor_burst_threshold) {
		sample->burst_data.active = true;
	} else {
		sample->burst_data.active = false;
	}
}

static void fls_def_sensor_burst_close(struct fls_def_sensor_sample *sample)
{
	ktime_t dur;

	if (!sample->burst_data.active) {
		memset(&(sample->burst_data), 0, sizeof(sample->burst_data));
		return;
	}

	dur = ktime_sub(sample->burst_data.last, sample->burst_data.start);

	if (sample->bursts == 0) {
		sample->burst_dur_sum = dur;
		sample->burst_dur_min = dur;
		sample->burst_dur_max = dur;

		sample->burst_sz_sum = sample->burst_data.sz;
		sample->burst_sz_min = sample->burst_data.sz;
		sample->burst_sz_max = sample->burst_data.sz;

		sample->bursts++;
		return;
	}

	sample->bursts++;

	sample->burst_dur_sum += dur;
	if (dur < sample->burst_dur_min) {
		sample->burst_dur_min = dur;
	} else if (dur > sample->burst_dur_max) {
		sample->burst_dur_max = dur;
	}

	sample->burst_sz_sum += sample->burst_data.sz;
	if (sample->burst_data.sz < sample->burst_sz_min) {
		sample->burst_sz_min = sample->burst_data.sz;
	} else if (sample->burst_data.sz > sample->burst_sz_max) {
		sample->burst_sz_max = sample->burst_data.sz;
	}

	memset(&(sample->burst_data), 0, sizeof(sample->burst_data));
}

static void fls_def_sensor_burst_record(struct fls_def_sensor_sample *sample, ktime_t now, uint32_t bytes)
{
	ktime_t delta;

	if (!sample->burst_data.start) {
		fls_def_sensor_burst_open(sample, now, bytes);
		return;
	}

	if (sample->burst_data.active) {
		delta = ktime_sub(now, sample->burst_data.last);
		if (delta >= fls_def_sensor_burst_long_intvl) {
			fls_def_sensor_burst_close(sample);
			fls_def_sensor_burst_open(sample, now, bytes);
			return;
		}

		sample->burst_data.sz += bytes;
		sample->burst_data.last = now;
		return;
	}

	delta = ktime_sub(now, sample->burst_data.start);
	if (delta >= fls_def_sensor_burst_short_intvl) {
		fls_def_sensor_burst_open(sample, now, bytes);
		return;
	}

	sample->burst_data.sz += bytes;
	sample->burst_data.last = now;
	if (sample->burst_data.sz > fls_def_sensor_burst_threshold) {
		sample->burst_data.active = true;
	}
}

static void fls_def_sensor_ipat_record(struct fls_def_sensor_sample *sample, ktime_t now)
{
	ktime_t delta;

	if (sample->packets == 0) {
		sample->last_packet_time = now;
		return;
	}

	delta = ktime_sub(now, sample->last_packet_time);
	sample->last_packet_time = now;

	if (sample->delta_sum == 0) {
		sample->delta_sum = delta;
		sample->delta_min = delta;
		sample->delta_max = delta;
		return;
	}

	sample->delta_sum += delta;
	if (delta < sample->delta_min) {
		sample->delta_min = delta;
	} else if (delta > sample->delta_max) {
		sample->delta_max = delta;
	}
}

void fls_def_sensor_packet_cb(void *app_data, struct fls_conn *conn, struct sk_buff *skb)
{
	ktime_t now;
	uint32_t sample_index;
	struct fls_def_sensor_sample *sample;
	uint32_t delay = fls_def_sensor_delay;
	uint32_t sample_length = fls_def_sensor_sample_length;

	if (fls_def_sensor_max_events == 0 || fls_def_sensor_sample_length == 0) {
		FLS_TRACE("%p Default sensor disabled.\n", conn);
		return;
	}

	if (!(conn->flags & FLS_CONNECTION_FLAG_DEF_ENABLE)) {
		FLS_TRACE("%p Statistics disabled.\n", conn);
		return;
	}

	now = ktime_get_boottime();
	if (!conn->stats.isd.first_packet_time) {
		FLS_INFO("%p First packet. t = %lld", conn, now);
		fls_debug_print_conn_info(conn);

		conn->stats.isd.first_packet_time = now;
		conn->stats.isd.event_start_time = now;
		conn->stats.isd.samples[0].sample_start_time = now;
	}

	if (!(conn->flags & FLS_CONNECTION_FLAG_DELAY_FINISHED)) {
		int64_t diff = ktime_to_ms(ktime_sub(now, conn->stats.isd.first_packet_time));
		if (diff < delay) {
			return;
		}

		conn->flags |= FLS_CONNECTION_FLAG_DELAY_FINISHED;
		FLS_INFO("%p Delay finished, starting data collection. t = %lld", conn, now);
		conn->stats.isd.first_packet_time = now;
		conn->stats.isd.event_start_time = now;
		conn->stats.isd.samples[0].sample_start_time = now;

		if (conn->reverse) {
			conn->reverse->flags |= FLS_CONNECTION_FLAG_DELAY_FINISHED;
			conn->reverse->stats.isd.first_packet_time = now;
			conn->reverse->stats.isd.event_start_time = now;
			conn->reverse->stats.isd.samples[0].sample_start_time = now;
		}

		fls_debug_print_conn_info(conn);
	}

	if (fls_def_sensor_dynamic_samples) {
		int64_t sample_diff;
		sample_index = conn->stats.isd.sample_index;
		sample_diff = ktime_to_ms(ktime_sub(now, conn->stats.isd.samples[sample_index].sample_start_time));

		/*
		 * If the time is past the end of the current sample, we need to start a new sample.
		 */
		if (sample_diff >= sample_length) {
			/*
			 * Check if sample exceeds watermark
			 */
			sample = &(conn->stats.isd.samples[sample_index]);
			if ((fls_def_sensor_pkts_hwm && sample->packets >= fls_def_sensor_pkts_hwm) || (fls_def_sensor_bytes_hwm && sample->bytes >= fls_def_sensor_bytes_hwm)) {
				FLS_INFO("%p HWM exceeded. pkts=%u pkt_hwm=%u, bytes=%u bytes_hwm=%u", conn, sample->packets, fls_def_sensor_pkts_hwm, sample->bytes, fls_def_sensor_bytes_hwm);
				conn->flags &= ~FLS_CONNECTION_FLAG_DEF_ENABLE;
				if (conn->reverse) {
					conn->reverse->flags &= ~FLS_CONNECTION_FLAG_DEF_ENABLE;
				}
			if (fls_def_sensor_burst) {
				fls_def_sensor_burst_close(&conn->stats.isd.samples[sample_index]);
			}

			sample_index += 1;
			FLS_TRACE("%p increased sample_index to %u", conn, sample_index);
			if (sample_index < fls_def_sensor_sample_count) {
				conn->stats.isd.samples[sample_index].sample_start_time = now;
				if (conn->reverse) {
					conn->reverse->stats.isd.samples[sample_index].sample_start_time = now;
				}
			}
		}
	} else {
		int32_t event_diff = ktime_to_ms(ktime_sub(now, conn->stats.isd.event_start_time));
		sample_index = event_diff / sample_length;

		if (fls_def_sensor_burst && sample_index > conn->stats.isd.sample_index) {
			fls_def_sensor_burst_close(&conn->stats.isd.samples[conn->stats.isd.sample_index]);
		}
	}

	/*
	 * If we've already generated enough samples, it's time to create a new event
	 */
	if (sample_index >= fls_def_sensor_sample_count) {
		int32_t abs_diff = ktime_to_ms(ktime_sub(now, conn->stats.isd.first_packet_time));
		uint32_t event_count;
		ktime_t event_start_new;

		/*
		 * Calculate the index of the event to be written.
		 */
		if (fls_def_sensor_dynamic_samples) {
			event_count = conn->stats.isd.events + 1;
			event_start_new = now;
		} else {
			event_count = abs_diff / FLS_DEF_SENSOR_TOTAL_TIME;
			event_start_new = ktime_add_ms(conn->stats.isd.event_start_time, (event_count - conn->stats.isd.events) * FLS_DEF_SENSOR_TOTAL_TIME);
		}

		/*
		 * If the calculated event index is greater than the index of the last generated event, generate a new event.
		 */
		if ((event_count > conn->stats.isd.events)) {
			struct fls_conn *reply = conn->reverse;
			fls_def_sensor_event_create(conn, now);
			conn->stats.isd.events = event_count;
			if (reply) {
				reply->stats.isd.events = event_count;
			}

			/*
			 * If we have a nonnegative max event count and have passed it, disable this connection and return.
			 */
			if ((fls_def_sensor_max_events >= 0) && (event_count >= fls_def_sensor_max_events)) {
				conn->flags &= ~FLS_CONNECTION_FLAG_DEF_ENABLE;
				if (reply) {
					reply->flags &= ~FLS_CONNECTION_FLAG_DEF_ENABLE;
				}

				return;
			}

			conn->stats.isd.event_start_time = event_start_new;
			if (conn->stats.isd.event_start_time > now) {
				FLS_WARN("Advanced time too far, now=%lli, event_start=%lli", now, conn->stats.isd.event_start_time);
			}

			if (reply) {
				reply->stats.isd.event_start_time = conn->stats.isd.event_start_time;
			}

			sample_index = 0;
			conn->stats.isd.samples[sample_index].sample_start_time = now;
			if (reply) {
				reply->stats.isd.samples[sample_index].sample_start_time = now;
			}
		}
	}

	conn->stats.isd.sample_index = sample_index;
	if (conn->reverse) {
		conn->reverse->stats.isd.sample_index = sample_index;
	}

	sample = &(conn->stats.isd.samples[sample_index]);

	if (fls_def_sensor_bytes) {
		fls_def_sensor_bytes_record(sample, skb->len);
	}
	if (fls_def_sensor_ipat) {
		fls_def_sensor_ipat_record(sample, now);
	}
	if (fls_def_sensor_burst) {
		fls_def_sensor_burst_record(sample, now, skb->len);
	}

	sample->packets++;
}

bool fls_def_sensor_init(struct fls_sensor_manager *fsm)
{
	fls_def_sensor_delay = FLS_DEF_SENSOR_DELAY_DEF;
	fls_def_sensor_sample_length = FLS_DEF_SENSOR_SAMPLE_LEN_DEF;
	fls_def_sensor_max_events = FLS_DEF_SENSOR_MAX_EVENTS_DEF;
	fls_def_sensor_dynamic_samples = FLS_DEF_SENSOR_DYNAMIC_SAMPLES_DEF;
	fls_def_sensor_sample_count = FLS_DEF_SENSOR_MAX_SAMPLE_COUNT;
	fls_def_sensor_bytes = 1;
	fls_def_sensor_ipat = 1;
	fls_def_sensor_bytes_hwm = 0;
	fls_def_sensor_pkts_hwm = 0;
	return fls_sensor_manager_register(fsm, fls_def_sensor_packet_cb, NULL);
}
