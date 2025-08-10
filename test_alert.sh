#!/bin/bash
echo "Test Alert Script Triggered!" >> /tmp/alert_test.log
echo "Args: $@" >> /tmp/alert_test.log
echo "Timestamp: $(date)" >> /tmp/alert_test.log
echo "-----" >> /tmp/alert_test.log
