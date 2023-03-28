#include "rtcp_fb.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "util.h"

rtcp_fb* rtcp_fb_create()
{
	rtcp_fb *packet=(rtcp_fb*)malloc(sizeof(rtcp_fb));
	if (packet) {
		memset(packet, 0, sizeof(rtcp_fb));
	}
	return packet;
}

void rtcp_fb_init(rtcp_fb* packet, uint16_t pt, uint8_t fmt)
{
	assert(packet != NULL);
	memset(packet, 0, sizeof(rtcp_fb));
	packet->header.app.version = 2;
	packet->header.app.pt = pt;
	packet->header.app.subtype = fmt;
	packet->header.app.length = 2;
}

void rtcp_fb_free(rtcp_fb* packet)
{
	assert(packet != NULL);
	if (packet->fci) {
		free(packet->fci);
	}
	free(packet);
}

size_t rtcp_fb_size(const rtcp_fb* packet)
{
	assert(packet != NULL);
	return 12 + packet->fci_size;
}

int rtcp_fb_set_fci(rtcp_fb* packet, const uint8_t* fci, uint16_t fci_size)
{
	assert(packet != NULL);
	if (packet->fci) {
		return -1;
	}

	packet->fci = malloc(fci_size);
	if (!packet->fci) {
		return -1;
	}
	memcpy(packet->fci, fci, fci_size);
	packet->fci_size = fci_size;
	packet->header.app.length = (uint16_t)((rtcp_fb_size(packet) / 4) - 1);
	return 0;
}

void rtcp_fb_free_fci(rtcp_fb* packet)
{
	assert(packet != NULL);
	if (packet->fci) {
		free(packet->fci);
	}
	packet->fci = NULL;
	packet->fci_size = 0;
	packet->header.app.length = 2;
	packet->header.app.p = 0;
}

int rtcp_fb_serialize(const rtcp_fb* packet, uint8_t* buffer, size_t size)
{
	assert(packet);
	const size_t packet_size = rtcp_fb_size(packet);
	if (size < packet_size) {
		return -1;
	}
	
	int r=rtcp_header_serialize(&packet->header, buffer, size);
	if (r < 0) {
		return r;
	}

	write_u32(buffer+4,packet->ssrc);
	write_u32(buffer+8,packet->dst_src);


	if (packet->fci_size > 0 && packet->fci) {
		memcpy(buffer + 12, packet->fci, packet->fci_size);
	}
	return packet_size;
}

int rtcp_fb_parse(rtcp_fb* packet, const uint8_t* buffer, size_t size)
{
	assert(packet != NULL);
	assert(buffer != NULL);

	uint16_t pt = rtcp_header_parse(&packet->header, buffer, size);
	if (pt != RTCP_RTPFB && pt != RTCP_PSFB) {
		return -1;
	}

	packet->ssrc = read_u32(buffer + 4);
	packet->dst_src = read_u32(buffer + 8);

	uint16_t fci_size = (packet->header.app.length - 2) * 4;
	uint16_t packet_size = 12 + fci_size;
	if (size < packet_size) {
		return -1;
	}
	
	int r = rtcp_fb_set_fci(packet, buffer + 12, fci_size);
	if (r < 0) {
		return r;
	}

	return packet_size;
}


int rtcp_rtpfb_nack_set(rtcp_fb* packet,uint16_t fst_pid, uint16_t nxt16_pid)
{
	assert(packet != NULL);
	uint8_t buffer[4];
	write_u16(buffer, fst_pid);
	write_u16(buffer + 2, nxt16_pid);
	return rtcp_fb_set_fci(packet, buffer, 4);
}

int rtcp_rtpfb_nack_get(const rtcp_fb* packet, uint16_t* fst_pid, uint16_t* nxt16_pid)
{
	assert(packet != NULL);
	assert(fst_pid != NULL);
	assert(nxt16_pid != NULL);
	if (packet->fci_size < 4) {
		return -1;
	}
	*fst_pid = read_u16(packet->fci);
	*nxt16_pid = read_u16(packet->fci + 2);
	return 0;
}


int rtcp_psfb_fir_set_items(rtcp_fb* packet, const rtcp_psfb_fir_item* items,size_t count)
{
	assert(packet != NULL);
	size_t size = count * sizeof(rtcp_psfb_fir_item);
	uint8_t* buffer=(uint8_t*)malloc(size);
	if (!buffer) {
		return -1;
	}
	for (size_t i = 0; i < count; ++i) {
		int pos = i * sizeof(rtcp_psfb_fir_item);
		write_u32(buffer+ pos, items[i].ssrc);
		buffer[pos + 4] = items[i].seq_nr;
		write_u24(buffer + pos +5, items[i].reserved);
	}
	rtcp_fb_set_fci(packet, buffer, size);
	free(buffer);
	return 0;
}

int rtcp_psfb_fir_find_item(const rtcp_fb* packet, uint32_t src, rtcp_psfb_fir_item* item)
{
	assert(packet != NULL);
	int count = rtcp_psfb_fir_item_count(packet);

	for (int i = 0; i < count; i++)
	{
		int pos = i * sizeof(rtcp_psfb_fir_item);
		uint32_t ssrc= read_u32(packet->fci + pos);
		if (ssrc == src)
		{
			item->ssrc = ssrc;
			item->seq_nr = packet->fci[pos + 4];
			item->reserved = read_u24(packet->fci + pos + 5);
			return i;
		}
	}

	return -1;
}

int rtcp_psfb_fir_get_item(const rtcp_fb* packet, int idx, rtcp_psfb_fir_item* item)
{
	assert(packet != NULL);
	int count = rtcp_psfb_fir_item_count(packet);
	if (idx >= count || idx < 0) {
		return -1;
	}

	int pos = idx * sizeof(rtcp_psfb_fir_item);
	item->ssrc = read_u32(packet->fci + pos);
	item->seq_nr = packet->fci[pos + 4];
	item->reserved = read_u24(packet->fci + pos + 5);
	return 0;
}
int rtcp_psfb_fir_item_count(const rtcp_fb* packet)
{
	assert(packet);
	return (packet->header.app.length - 2) / 2;
}
